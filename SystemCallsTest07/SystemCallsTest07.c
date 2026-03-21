#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"

/*********************************************************************************
*
* SystemCallsTest07
*
* PURPOSE:
*   Tests that multiple processes can be simultaneously blocked on the same
*   semaphore and that multiple SemV operations correctly release them one
*   at a time. Three children each perform one SemP on a semaphore with
*   initial value 0 and will all block. A fourth child performs three SemV
*   operations to release all three blocked children. The parent exits
*   without waiting, exercising the orphan cleanup path for all children.
*
* EXPECTED BEHAVIOR:
*   - Children 1, 2, and 3 all block on SemP.
*   - Child 4 calls SemV three times, releasing one blocker per call.
*   - All three blocked children unblock and exit with status 9.
*   - Parent exits with status 8 without calling Wait.
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemP (multiple blockers), SemV (multiple releases), Spawn, Exit
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
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 0 Semps - 3 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 3, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
