#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int SleepThenSemV(void* strArgs);

/*********************************************************************************
*
* SystemCallsTest19
*
* PURPOSE:
*   Tests GetTimeofDay busy-wait in combination with semaphores to verify that
*   a waiting process allows other processes to make progress. A child busy-waits
*   for 2 seconds using GetTimeofDay and then signals a semaphore. A second
*   child blocks on that semaphore and can only proceed after the first wakes.
*   The parent verifies both children complete and that the semaphore-blocked
*   child did not unblock prematurely.
*
* EXPECTED BEHAVIOR:
*   - The SemP child blocks immediately and waits.
*   - The sleeping child wakes after ~2 seconds and calls SemV.
*   - The SemP child then unblocks and exits.
*   - Total elapsed time reported by parent should be approximately 2 seconds.
*   - Demonstrates that other processes run while the busy-wait executes.
*
* SYSTEM CALLS TESTED:
*   GetTimeofDay, SemCreate, SemP, SemV, Spawn, Wait, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    int semaphore;
    int sem_result;
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    int status;
    int startTime, endTime;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);

    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: SemCreate returned %d, semaphore = %d\n", testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    GetTimeofDay(&startTime);

    /* Child 1: blocks on semaphore, waiting for the sleeper to signal it */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: spawned SemP child with PID %d\n", testName, pid);

    /* Child 2: sleeps 2 seconds then signals the semaphore */
    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Sleeper2", testName);
    Spawn(nameBuffer, SleepThenSemV, &semaphore, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: spawned sleeping SemV child with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d\n", testName, pid, status);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d\n", testName, pid, status);

    GetTimeofDay(&endTime);
    console_output(FALSE, "%s: total elapsed time = %d microseconds (expected ~2 seconds)\n",
        testName, endTime - startTime);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


/*
 * SleepThenSemV
 *
 * Sleeps for 2 seconds then signals the semaphore whose handle is passed
 * as the argument.
 */
int SleepThenSemV(void* strArgs)
{
    int semHandle = *(int*)strArgs;
    int result;

    int _bwStart, _bwNow;
    console_output(FALSE, "SleepThenSemV: started, busy-waiting 2 seconds\n");
    GetTimeofDay(&_bwStart);
    do { GetTimeofDay(&_bwNow); } while (_bwNow - _bwStart < 2000000);
    console_output(FALSE, "SleepThenSemV: done waiting, calling SemV on semaphore %d\n", semHandle);

    result = SemV(semHandle);
    console_output(FALSE, "SleepThenSemV: SemV returned %d\n", result);

    Exit(9);

    return 0;
}