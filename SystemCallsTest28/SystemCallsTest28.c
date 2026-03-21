#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int BlockOnSemAndRecord(char* strArgs);
int ReleaseInOrder(char* strArgs);

/*********************************************************************************
*
* SystemCallsTest28
*
* PURPOSE:
*   Tests that processes blocked on a semaphore are unblocked in FIFO (first-in,
*   first-out) order. When multiple processes are all waiting on the same
*   semaphore and SemV is called repeatedly, the process that blocked first
*   should be the first to be released.
*
*   Four waiters are spawned at equal priority, all blocking on the same
*   semaphore initialized to 0. A fifth child (the releaser) calls SemV four
*   times with a brief busy-wait between each to allow the newly unblocked
*   waiter to run. Each waiter's spawn order is encoded in its name string
*   (e.g. "...Waiter1:1") and compared against a shared release counter.
*
*   NOTE: FIFO ordering is only guaranteed when all waiters have equal priority.
*   All waiters use priority 3 for this reason.
*
* EXPECTED BEHAVIOR:
*   - Waiters unblock in the same order they were spawned (FIFO).
*   - Each SemV releases exactly one waiter.
*   - Exit statuses of waiters reflect their unblock sequence (1, 2, 3, 4).
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemP (blocking, ordered release), SemV, Spawn, Wait, Exit
*
*********************************************************************************/

static int sharedSemaphore;
static int releaseSequence = 0;

int SystemCallsEntryPoint(void* pArgs)
{
    int sem_result;
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int pid;
    int status;
    int i;

    console_output(FALSE, "\n%s: started\n", testName);

    sem_result = SemCreate(0, &sharedSemaphore);
    console_output(FALSE, "%s: SemCreate(0) returned %d, semaphore = %d\n",
        testName, sem_result, sharedSemaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* Spawn four waiters at equal priority.
       Spawn order is encoded in the name: "...Waiter1:1", "...Waiter2:2", etc. */
    for (i = 1; i <= 4; i++) {
        snprintf(nameBuffer, sizeof(nameBuffer), "%s-Waiter%d:%d", testName, i, i);
        Spawn(nameBuffer, BlockOnSemAndRecord, nameBuffer,
            THREADS_MIN_STACK_SIZE, 3, &pid);
        console_output(FALSE, "%s: spawned waiter %d with PID %d\n",
            testName, i, pid);
    }

    /* Releaser at lower priority so all waiters block first */
    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Releaser", testName);
    Spawn(nameBuffer, ReleaseInOrder, nameBuffer, THREADS_MIN_STACK_SIZE, 2, &pid);
    console_output(FALSE, "%s: spawned releaser with PID %d\n", testName, pid);

    for (i = 0; i < 5; i++) {
        Wait(&pid, &status);
        console_output(FALSE, "%s: Wait returned PID %d with status %d\n",
            testName, pid, status);
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


/*
 * BlockOnSemAndRecord
 *
 * Parses spawn order from name string (e.g. "...Waiter1:1" -> spawnOrder=1).
 * Blocks on semaphore, records release sequence, compares to spawn order.
 */
int BlockOnSemAndRecord(char* strArgs)
{
    int spawnOrder = 0;
    int mySequence;
    int result;
    char* separator = strchr(strArgs, ':');

    if (separator != NULL) {
        separator[0] = '\0';
        separator++;
        sscanf(separator, "%d", &spawnOrder);
    }

    console_output(FALSE, "%s: blocking on semaphore %d (spawn order %d)\n",
        strArgs, sharedSemaphore, spawnOrder);

    result = SemP(sharedSemaphore);

    mySequence = ++releaseSequence;

    console_output(FALSE, "%s: unblocked, release sequence = %d, spawn order = %d %s\n",
        strArgs, mySequence, spawnOrder,
        mySequence == spawnOrder ? "PASS" : "FAIL");

    Exit(mySequence);

    return 0;
}


/*
 * ReleaseInOrder
 *
 * Calls SemV four times with a brief busy-wait between each to allow the
 * newly unblocked waiter to run before the next SemV is issued.
 */
int ReleaseInOrder(char* strArgs)
{
    int i, result;
    int _bwStart, _bwNow;

    console_output(FALSE, "%s: releaser started\n", strArgs);

    for (i = 1; i <= 4; i++) {
        console_output(FALSE, "%s: calling SemV %d of 4\n", strArgs, i);
        result = SemV(sharedSemaphore);
        console_output(FALSE, "%s: SemV returned %d\n", strArgs, result);

        /* Brief busy-wait so unblocked waiter runs before next SemV */
        GetTimeofDay(&_bwStart);
        do { GetTimeofDay(&_bwNow); } while (_bwNow - _bwStart < 100000);
    }

    console_output(FALSE, "%s: releaser done\n", strArgs);
    Exit(0);

    return 0;
}