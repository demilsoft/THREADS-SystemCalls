#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest16
*
* PURPOSE:
*   Tests a complex semaphore scenario where the parent process participates
*   directly in semaphore operations alongside its children. A semaphore is
*   created with initial value 3, then the parent immediately calls SemP to
*   reduce it to 2. Child 1 performs 5 SemP operations (will exhaust the
*   semaphore and block after the 2nd). Child 2 performs 2 SemV operations
*   to partially unblock Child 1. The parent then calls SemV to provide the
*   final signal needed for Child 1 to complete. The parent waits for both
*   children. This tests the interaction of parent and child on shared
*   semaphore state and the counting behavior across multiple operations.
*
* EXPECTED BEHAVIOR:
*   - Parent SemP reduces semaphore from 3 to 2.
*   - Child 1 does 2 SemPs successfully, then blocks on the 3rd.
*   - Child 2 does 2 SemVs (semaphore goes to 2), unblocking Child 1 partially.
*   - Parent SemV provides the last signal needed.
*   - Both children eventually complete and exit with status 9.
*   - Both Wait calls return successfully.
*   - Parent exits with status 8.
*
* SYSTEM CALLS TESTED:
*   SemCreate (initial value > 0), SemP, SemV (parent and child), Spawn, Wait, Exit
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
    int status;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(3, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    SemP(semaphore);

    /* 5 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 5, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 0 Semps - 3 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 2, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    SemV(semaphore);
    console_output(FALSE, "%s: after Semv\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);
    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
