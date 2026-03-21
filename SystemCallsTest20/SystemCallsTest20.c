#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest20
*
* PURPOSE:
*   Tests error handling for invalid semaphore handles. The spec requires that
*   SemP, SemV, and SemFree all return -1 when called with an invalid semaphore
*   identifier. This test exercises several invalid handle scenarios:
*
*     1. A negative handle (-1)
*     2. A handle beyond the valid range (MAXSEMS + 10)
*     3. A handle that was never created (a valid-range index that was never
*        returned by SemCreate)
*
*   None of these calls should crash, hang, or cause undefined behavior. Each
*   must return -1 cleanly.
*
* EXPECTED BEHAVIOR:
*   - Every SemP, SemV, and SemFree call below returns -1.
*   - The process does not block on any of these calls.
*   - The process exits normally with status 8.
*
* SYSTEM CALLS TESTED:
*   SemP, SemV, SemFree (error paths), Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int result;

    console_output(FALSE, "\n%s: started\n", testName);

    /* --- Negative handle --- */
    console_output(FALSE, "%s: Testing negative handle (-1)\n", testName);

    result = SemP(-1);
    console_output(FALSE, "%s: SemP(-1) returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    result = SemV(-1);
    console_output(FALSE, "%s: SemV(-1) returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    result = SemFree(-1);
    console_output(FALSE, "%s: SemFree(-1) returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    /* --- Out-of-range handle --- */
    console_output(FALSE, "%s: Testing out-of-range handle (%d)\n", testName, MAXSEMS + 10);

    result = SemP(MAXSEMS + 10);
    console_output(FALSE, "%s: SemP(%d) returned %d (expected -1) %s\n",
                   testName, MAXSEMS + 10, result, result == -1 ? "PASS" : "FAIL");

    result = SemV(MAXSEMS + 10);
    console_output(FALSE, "%s: SemV(%d) returned %d (expected -1) %s\n",
                   testName, MAXSEMS + 10, result, result == -1 ? "PASS" : "FAIL");

    result = SemFree(MAXSEMS + 10);
    console_output(FALSE, "%s: SemFree(%d) returned %d (expected -1) %s\n",
                   testName, MAXSEMS + 10, result, result == -1 ? "PASS" : "FAIL");

    /* --- Valid-range but never-created handle ---
       Use handle 0 directly without having called SemCreate first.
       This assumes the semaphore table is cleared at startup. */
    console_output(FALSE, "%s: Testing valid-range but uncreated handle (0)\n", testName);

    result = SemP(0);
    console_output(FALSE, "%s: SemP(0) [uncreated] returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    result = SemV(0);
    console_output(FALSE, "%s: SemV(0) [uncreated] returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    result = SemFree(0);
    console_output(FALSE, "%s: SemFree(0) [uncreated] returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}
