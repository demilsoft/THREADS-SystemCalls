#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"

/*********************************************************************************
*
* SystemCallsTest00
*
* PURPOSE:
*   Verifies that user-level processes run in user mode, not kernel mode.
*   This is the most fundamental correctness check for the System Calls project.
*   The PSR (Processor Status Register) is read directly and the kernel mode
*   bit is inspected. If the process is incorrectly running in kernel mode,
*   the interrupt-based system call boundary has not been established properly.
*
* EXPECTED BEHAVIOR:
*   - get_psr() returns a value with PSR_KERNEL_MODE bit clear.
*   - "Kernel is in user mode, TEST PASSED" is printed.
*   - If the kernel mode bit is set, "TEST FAILED" is printed.
*
* SYSTEM CALLS TESTED:
*   Exit (implicit process termination path), PSR inspection via get_psr()
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    uint32_t psr;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    psr = get_psr();

    /* We should be in user mode here */
    if (psr & PSR_KERNEL_MODE)
    {
        console_output(FALSE, "%s: Kernel is in kernel mode, TEST FAILED.\n", testName);
    }
    else
    {
        console_output(FALSE, "%s: Kernel is in user mode, TEST PASSED.\n", testName);
    }

    Exit(0);

    return 0;
}
