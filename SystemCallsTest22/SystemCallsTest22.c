#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest22
*
* PURPOSE:
*   Tests the behavior of Wait when the calling process has no children.
*   This is an error condition. The spec implies Wait should return an error
*   rather than blocking indefinitely when there are no child processes to
*   wait for.
*
*   A robust implementation must:
*     1. Not block forever (would hang the test).
*     2. Return a meaningful error value (typically -1 or -2) indicating
*        that no children exist.
*     3. Leave the pid and status output parameters in a defined state.
*
*   This test is intentionally simple: no Spawn is called before Wait.
*
* EXPECTED BEHAVIOR:
*   - Wait returns immediately with a non-zero / negative return value.
*   - The process does not hang.
*   - The process exits normally with status 8.
*
* NOTE:
*   If this test hangs, the implementation is blocking on Wait when it
*   should be returning an error — a clear bug.
*
* SYSTEM CALLS TESTED:
*   Wait (error path), Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int pid = -1;
    int status = -1;
    int result;

    console_output(FALSE, "\n%s: started\n", testName);
    console_output(FALSE, "%s: Calling Wait with no children spawned\n", testName);

    result = Wait(&pid, &status);

    /* If we reach here the call returned, which is correct.
       If the test hangs above, the implementation is blocking incorrectly. */
    console_output(FALSE, "%s: Wait returned %d, pid = %d, status = %d\n",
                   testName, result, pid, status);

    if (result < 0)
        console_output(FALSE, "%s: Wait correctly returned error code %d PASS\n", testName, result);
    else
        console_output(FALSE, "%s: Wait returned %d -- expected a negative error code FAIL\n", testName, result);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}
