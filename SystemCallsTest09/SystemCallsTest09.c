#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest09
*
* PURPOSE:
*   Tests the SemFree system call when processes are blocked on the semaphore
*   being freed. Three children each perform SemP on a semaphore with initial
*   value 0 and will all block. A fourth child calls SemFree on that semaphore.
*   Per the spec, SemFree must unblock all waiting processes, which must then
*   exit immediately with error code 1. This verifies that SemFree correctly
*   handles the case where blocked processes exist at the time of the free.
*
* EXPECTED BEHAVIOR:
*   - Children 1, 2, and 3 block on SemP.
*   - Child 4 calls SemFree, which returns 1 (processes were waiting).
*   - All three blocked children are unblocked and exit with status 1.
*   - Child 4 exits with status 9.
*   - Parent exits with status 8 without waiting.
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemP (blocking), SemFree (with blocked processes), Spawn, Exit
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
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 5, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 5, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 5, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 0 Semps - 0 Semvs - SEM FREE */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 0, SYSCALL_TEST_OPTION_SEMFREE);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
