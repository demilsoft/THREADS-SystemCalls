#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest23
*
* PURPOSE:
*   Tests the behavior of Wait when called more times than there are children.
*   This extends Test22 by first successfully waiting on all children, then
*   calling Wait one additional time when no children remain.
*
*   The first two Wait calls should succeed normally. The third Wait call
*   must return an error rather than blocking indefinitely. This verifies
*   that the implementation correctly tracks child process count and does
*   not allow Wait to block when the child list is exhausted.
*
* EXPECTED BEHAVIOR:
*   - Spawn 2 children successfully.
*   - First Wait returns successfully for one child.
*   - Second Wait returns successfully for the other child.
*   - Third Wait (excess) returns a negative error code immediately.
*   - The process does not hang on the third Wait.
*
* NOTE:
*   If the third Wait hangs, the implementation is not correctly tracking
*   the number of living/uncollected child processes.
*
* SYSTEM CALLS TESTED:
*   Spawn, Wait (success path and error path), Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    int status;
    int result;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);

    /* Spawn two simple children that exit immediately */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, 0, 0, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned child 1 with PID %d\n", testName, pid);

    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, 0, 0, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned child 2 with PID %d\n", testName, pid);

    /* First Wait - should succeed */
    result = Wait(&pid, &status);
    console_output(FALSE, "%s: Wait 1 returned %d for PID %d with status %d\n",
                   testName, result, pid, status);

    /* Second Wait - should succeed */
    result = Wait(&pid, &status);
    console_output(FALSE, "%s: Wait 2 returned %d for PID %d with status %d\n",
                   testName, result, pid, status);

    /* Third Wait - no children remain, must return error immediately */
    console_output(FALSE, "%s: Calling Wait a third time (no children remain)\n", testName);
    result = Wait(&pid, &status);
    console_output(FALSE, "%s: Wait 3 returned %d (expected negative error code) %s\n",
                   testName, result, result < 0 ? "PASS" : "FAIL");

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}
