#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest21
*
* PURPOSE:
*   Tests the behavior of SemFree when called on a semaphore that has already
*   been freed. This is a double-free scenario. The spec does not explicitly
*   define behavior for this case, but a robust implementation must not crash,
*   hang, or corrupt internal state. The expected return value is -1, since
*   the handle is no longer valid after the first SemFree.
*
*   Additionally, this test verifies that after a semaphore is freed and its
*   slot is reclaimed by a subsequent SemCreate, the new semaphore operates
*   correctly. This confirms that the double-free did not corrupt the semaphore
*   table.
*
* EXPECTED BEHAVIOR:
*   - First SemCreate succeeds, returns handle >= 0.
*   - First SemFree returns 0 (success, no waiters).
*   - Second SemFree on the same handle returns -1 (already freed).
*   - A new SemCreate succeeds (slot was cleanly reclaimed).
*   - SemP and SemV on the new semaphore operate correctly.
*
* SYSTEM CALLS TESTED:
*   SemCreate, SemFree (double-free), SemP, SemV, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int semaphore;
    int semaphore2;
    int sem_result;
    int result;

    console_output(FALSE, "\n%s: started\n", testName);

    /* Create a semaphore with initial value 1 */
    sem_result = SemCreate(1, &semaphore);
    console_output(FALSE, "%s: SemCreate returned %d, semaphore = %d\n",
                   testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* Verify it works normally before freeing */
    result = SemP(semaphore);
    console_output(FALSE, "%s: SemP returned %d (expected 0)\n", testName, result);

    result = SemV(semaphore);
    console_output(FALSE, "%s: SemV returned %d (expected 0)\n", testName, result);

    /* First free - should succeed with return value 0 (no waiters) */
    result = SemFree(semaphore);
    console_output(FALSE, "%s: First SemFree returned %d (expected 0) %s\n",
                   testName, result, result == 0 ? "PASS" : "FAIL");

    /* Second free - should return -1, handle is no longer valid */
    result = SemFree(semaphore);
    console_output(FALSE, "%s: Second SemFree returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    /* Verify SemP and SemV also reject the freed handle */
    result = SemP(semaphore);
    console_output(FALSE, "%s: SemP on freed semaphore returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    result = SemV(semaphore);
    console_output(FALSE, "%s: SemV on freed semaphore returned %d (expected -1) %s\n",
                   testName, result, result == -1 ? "PASS" : "FAIL");

    /* Verify the slot was cleanly reclaimed - new SemCreate should succeed */
    sem_result = SemCreate(1, &semaphore2);
    console_output(FALSE, "%s: SemCreate after double-free returned %d, semaphore = %d (expected success) %s\n",
                   testName, sem_result, semaphore2, sem_result == 0 ? "PASS" : "FAIL");

    if (sem_result == 0) {
        result = SemP(semaphore2);
        console_output(FALSE, "%s: SemP on new semaphore returned %d (expected 0) %s\n",
                       testName, result, result == 0 ? "PASS" : "FAIL");

        result = SemV(semaphore2);
        console_output(FALSE, "%s: SemV on new semaphore returned %d (expected 0) %s\n",
                       testName, result, result == 0 ? "PASS" : "FAIL");

        SemFree(semaphore2);
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}
