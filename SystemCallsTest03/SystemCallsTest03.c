#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int SpawnOne(void* strArgs);

/*********************************************************************************
*
* SystemCallsTest03
*
* PURPOSE:
*   Tests a two-level process hierarchy where the root process spawns an
*   intermediate child, which in turn spawns a grandchild. This verifies
*   that Spawn works correctly from within a child process (not just from
*   the initial entry point), and that the process tree can be at least
*   three levels deep. The root exits without waiting; the intermediate
*   exits after spawning; the grandchild does no semaphore work.
*
* EXPECTED BEHAVIOR:
*   - Root spawns Child1 (intermediate) successfully.
*   - Child1 spawns its own child (grandchild) successfully.
*   - Grandchild starts and exits with status 9.
*   - Child1 exits with status 7.
*   - Root exits with status 8 without waiting.
*
* SYSTEM CALLS TESTED:
*   Spawn (from child context), Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int status = -1, pid1, kidpid = -1;
    char nameBuffer[512];

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    /* Use the -Child naming convention for the child process name. */
    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child1", testName);

    Spawn(nameBuffer, SpawnOne, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid1);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid1);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


int SpawnOne(void* strArgs)
{
    int kidpid;
    int status = 0xff;
    char nameBuffer[1024];
    int childId = 0;
    int pid;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", strArgs);

    console_output(FALSE, "\n%s: Spawning one child\n", strArgs);

    /* 0 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), strArgs, ++childId, 0, 0, 0, 0);

    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 5, &pid);

    console_output(FALSE, "%s: after spawn of child with PID %d\n", strArgs, pid);
    console_output(FALSE, "%s: finished\n", strArgs);


    Exit(7);

    return 0;

}
