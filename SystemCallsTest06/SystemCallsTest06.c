#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"

/*********************************************************************************
*
* SystemCallsTest06
*
* PURPOSE:
*   Tests basic semaphore blocking and unblocking between two child processes.
*   A semaphore is created with initial value 0. Child 1 attempts SemP
*   immediately and must block since the value is 0. Child 2 calls SemV
*   to increment the semaphore, which unblocks Child 1. The parent calls
*   Wait twice to collect both children. This is the fundamental producer/
*   consumer pattern and verifies the core SemP/SemV interaction.
*
* EXPECTED BEHAVIOR:
*   - Child 1 blocks on SemP until Child 2 calls SemV.
*   - Child 2 completes SemV and exits.
*   - Child 1 unblocks, completes, and exits with status 9.
*   - Both Wait calls return successfully.
*   - Parent exits with status 8.
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemP (blocking), SemV (unblocking), Spawn, Wait, Exit
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

    /* 0 Semps - 1 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 1, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
