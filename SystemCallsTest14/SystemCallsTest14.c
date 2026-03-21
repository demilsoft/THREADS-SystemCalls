#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest14
*
* PURPOSE:
*   Basic wait-and-terminate test. Spawns a single child that performs no
*   semaphore operations and exits, then the parent calls Wait to collect it.
*   This is the simplest end-to-end test of the Spawn/Wait/Exit cycle and
*   verifies that Wait correctly blocks until the child exits and returns
*   both the child's PID and exit status to the parent.
*
* EXPECTED BEHAVIOR:
*   - Child starts, performs 0 semaphore operations, exits with status 9.
*   - Parent's Wait returns the child's PID and status 9.
*   - Parent prints the returned PID and status.
*   - Parent exits with status 8.
*
* SYSTEM CALLS TESTED:
*   Spawn, Wait, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    int status;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    /* 0 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, 0, 0, 0, 0);

    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 2, &pid);

    console_output(FALSE, "%s: after spawn of %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
