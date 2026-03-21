#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest01
*
* PURPOSE:
*   Basic smoke test for the Spawn system call. Spawns a single child process
*   that performs no semaphore operations and exits immediately. Verifies that
*   Spawn returns a valid PID and that a user-level child process can be created
*   and run without error. The parent exits without calling Wait, which also
*   exercises the orphan path for a child that has already exited.
*
* EXPECTED BEHAVIOR:
*   - Spawn returns a positive PID for the child.
*   - Child starts, performs 0 SemP and 0 SemV operations, exits with status 9.
*   - Parent exits with status 8.
*
* SYSTEM CALLS TESTED:
*   Spawn, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    /* 0 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, 0, 0, 0, 0);

    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 2, &pid);
    
    console_output(FALSE, "%s: after spawn of %d\n", testName, pid);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
