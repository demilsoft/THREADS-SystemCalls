#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"

/*********************************************************************************
*
* SystemCallsTest02
*
* PURPOSE:
*   Tests that a user-level process which returns from its entry point function
*   (rather than explicitly calling Exit) is handled correctly by the kernel.
*   The process returns -1 from SystemCallsEntryPoint. The kernel must treat
*   this return as an implicit Exit with the returned value as the status code.
*   This verifies that the system call wrapper around the user entry point
*   properly intercepts the return and calls Exit on the process's behalf.
*
* EXPECTED BEHAVIOR:
*   - Process returns -1 without calling Exit.
*   - System does not crash or hang.
*   - Kernel treats the return as a clean process termination.
*
* SYSTEM CALLS TESTED:
*   Exit (implicit, via return from entry point)
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);

    /* Output a message and return.  This should result in a proper exit. */
    console_output(FALSE, "\n%s: started and returning -1.\n", testName);

    return -1;
}
