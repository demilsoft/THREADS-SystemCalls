#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int WaitForSemWithPriority(char* strArgs);
int SignalSemaphore(char* strArgs);

/*********************************************************************************
*
* SystemCallsTest26
*
* PURPOSE:
*   Tests the interaction between process priority and semaphore wake-up order.
*   When multiple processes of different priorities are all blocked on the same
*   semaphore, the highest-priority process should be unblocked first when
*   SemV is called.
*
*   IMPORTANT - THREADS PRIORITY CONVENTION:
*   In this system, HIGHER numeric value = HIGHER scheduling priority.
*     LOWEST_PRIORITY  = 0  (lowest scheduling priority, reserved for watchdog)
*     HIGHEST_PRIORITY = 5  (highest scheduling priority)
*   This is the OPPOSITE of many OS textbook conventions.
*
*   Three waiters are spawned at priorities 4 (highest), 3, and 2 (lowest),
*   all blocking on the same semaphore initialized to 0. A releaser at
*   priority 1 calls SemV three times. The expected unblock order is:
*     1. Priority 4 (highest) first
*     2. Priority 3 second
*     3. Priority 2 (lowest) last
*
*   Each waiter's priority is encoded in its name string and used as its
*   exit status so the parent can verify the unblock order. Waiters are
*   spawned lowest-to-highest so all block before the releaser runs.
*
*   If the implementation uses FIFO regardless of priority, this test will
*   reveal it by returning them in spawn order (2, 3, 4) instead of
*   priority order (4, 3, 2).
*
* EXPECTED BEHAVIOR:
*   - First child to complete has priority 4 (status 4).
*   - Second child to complete has priority 3 (status 3).
*   - Third child to complete has priority 2 (status 2).
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemP (priority-ordered release), SemV, Spawn, Wait, Exit
*
*********************************************************************************/

static int prioritySemaphore;

int SystemCallsEntryPoint(void* pArgs)
{
    int sem_result;
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int pid;
    int status;
    int i;
    /* Spawn lowest priority first so all block before releaser runs */
    int priorities[3] = { 4, 3, 2 };

    console_output(FALSE, "\n%s: started\n", testName);

    sem_result = SemCreate(0, &prioritySemaphore);
    console_output(FALSE, "%s: SemCreate(0) returned %d, semaphore = %d\n",
        testName, sem_result, prioritySemaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* Spawn waiters lowest-to-highest priority, encoding priority in name */
    for (i = 0; i < 3; i++) {
        snprintf(nameBuffer, sizeof(nameBuffer), "%s-Pri%d:%d",
            testName, priorities[i], priorities[i]);
        Spawn(nameBuffer, WaitForSemWithPriority, nameBuffer,
            THREADS_MIN_STACK_SIZE, priorities[i], &pid);
        console_output(FALSE, "%s: spawned priority-%d waiter with PID %d\n",
            testName, priorities[i], pid);
    }

    /* Releaser at priority 1 (highest user priority) -- runs after all waiters block */
    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Releaser", testName);
    Spawn(nameBuffer, SignalSemaphore, nameBuffer, THREADS_MIN_STACK_SIZE, 1, &pid);
    console_output(FALSE, "%s: spawned releaser with PID %d at priority 1\n",
        testName, pid);

    console_output(FALSE, "%s: Waiting for children (expect priority order: 4, 3, 2)\n",
        testName);

    for (i = 0; i < 4; i++) {
        Wait(&pid, &status);
        console_output(FALSE, "%s: Wait returned PID %d with status %d\n",
            testName, pid, status);
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


/*
 * WaitForSemWithPriority
 *
 * Parses priority from name string (e.g. "...Pri3:3" -> priority=3).
 * Blocks on semaphore then exits with priority as status so parent
 * can verify unblock order.
 */
int WaitForSemWithPriority(char* strArgs)
{
    int priority = 0;
    int result;
    char* separator = strchr(strArgs, ':');

    if (separator != NULL) {
        separator[0] = '\0';
        separator++;
        sscanf(separator, "%d", &priority);
    }

    console_output(FALSE, "%s: (priority %d) blocking on semaphore %d\n",
        strArgs, priority, prioritySemaphore);

    result = SemP(prioritySemaphore);

    console_output(FALSE, "%s: (priority %d) unblocked, SemP returned %d\n",
        strArgs, priority, result);

    Exit(priority);

    return 0;
}


/*
 * SignalSemaphore
 *
 * Calls SemV three times with a brief busy-wait between each to allow
 * the scheduler to run the highest-priority unblocked waiter before
 * the next signal is issued.
 */
int SignalSemaphore(char* strArgs)
{
    int i, result;
    int _bwStart, _bwNow;

    console_output(FALSE, "%s: started, will SemV 3 times\n", strArgs);

    for (i = 1; i <= 3; i++) {
        result = SemV(prioritySemaphore);
        console_output(FALSE, "%s: SemV %d returned %d\n", strArgs, i, result);

        /* Brief busy-wait to let unblocked waiter run */
        GetTimeofDay(&_bwStart);
        do { GetTimeofDay(&_bwNow); } while (_bwNow - _bwStart < 100000);
    }

    console_output(FALSE, "%s: done\n", strArgs);
    Exit(0);

    return 0;
}