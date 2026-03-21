#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest04
*
* PURPOSE:
*   Basic smoke test for the SemCreate system call. Creates two semaphores
*   sequentially and verifies that each returns a success code (0) and a
*   valid semaphore handle. This confirms that the semaphore table is
*   initialized correctly and that successive SemCreate calls return distinct
*   handles. No semaphore operations (SemP/SemV) are performed.
*
* EXPECTED BEHAVIOR:
*   - First SemCreate(0) returns 0 with a valid handle (>= 0).
*   - Second SemCreate(0) returns 0 with a different valid handle.
*   - Process exits with status 8.
*
* SYSTEM CALLS TESTED:
*   SemCreate, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    int semaphore;
    int sem_result;
    char* testName = GetTestName(__FILE__);

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);

    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);

    Exit(8);

    return 0;
}
