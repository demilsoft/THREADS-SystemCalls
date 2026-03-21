#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest05
*
* PURPOSE:
*   Tests the upper boundary of semaphore creation. Attempts to create
*   MAXSEMS + 2 semaphores in a loop and records the return value at each
*   index. The first MAXSEMS calls must succeed (return 0). Once the semaphore
*   table is full, subsequent calls must return -1. This verifies that the
*   kernel correctly enforces the MAXSEMS limit and does not overflow its
*   internal semaphore table.
*
* EXPECTED BEHAVIOR:
*   - Indices 0 through MAXSEMS-1: SemCreate returns 0.
*   - Index MAXSEMS and MAXSEMS+1: SemCreate returns -1.
*   - No crash or memory corruption occurs at the boundary.
*
* SYSTEM CALLS TESTED:
*   SemCreate (boundary/limit), Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    int semaphore;
    int sem_result;
    int i;
    char* testName = GetTestName(__FILE__);

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    for (i = 0; i < MAXSEMS + 2; i++) {
        sem_result = SemCreate(0, &semaphore);
        console_output(FALSE, "%s: SemCreate returned %2d at index %3d\n", testName, sem_result, i);
    }
    Exit(8);

    return 0;
}
