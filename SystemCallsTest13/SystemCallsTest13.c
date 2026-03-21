#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest13
*
* PURPOSE:
*   Tests SemFree return value when processes are blocked on the semaphore.
*   A semaphore is created with initial value 1, then immediately decremented
*   to 0 by the parent via SemP. Two children are spawned that each attempt
*   SemP followed by SemV (SEMP_FIRST option), causing both to block since
*   the semaphore value is 0. The parent then calls SemFree. Per the spec,
*   SemFree must return 1 when there are blocked processes, and must unblock
*   all of them. A return value other than 1 is treated as a test failure.
*
* EXPECTED BEHAVIOR:
*   - Parent's SemP succeeds, depleting the semaphore.
*   - Both children block on their SemP calls.
*   - SemFree returns 1 (blocked processes present) -- prints confirmation.
*   - Any other return value prints "TEST FAILED".
*   - Parent exits with status 8.
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemP, SemFree (return value with blocked processes), Spawn, Exit
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
    int result;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(1, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* hold the semaphore */
    SemP(semaphore);

    /* 1 Semps - 1 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 1, SYSCALL_TEST_OPTION_SEMP_FIRST);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 1 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 1, SYSCALL_TEST_OPTION_SEMP_FIRST);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    result = SemFree(semaphore);
    if (result == 1)
    {
        console_output(FALSE, "%s: After SemFree with processes blocked\n", testName);
    }
    else 
    {
        console_output(FALSE, "%s: TEST FAILED...wrong value returned from SemFree\n", testName);
        stop(1);
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
