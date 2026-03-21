#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"

/*********************************************************************************
*
* SystemCallsTest08
*
* PURPOSE:
*   Tests semaphore table capacity management including overflow detection
*   and slot reclamation after SemFree. Creates exactly MAXSEMS semaphores
*   (which must all succeed), then attempts one more (which must return -1).
*   Frees semaphore at index 105, then creates a new one, which must succeed
*   by reusing the freed slot. This verifies that the semaphore table correctly
*   tracks free slots and reclaims them after SemFree.
*
* EXPECTED BEHAVIOR:
*   - All MAXSEMS creates succeed silently.
*   - Create at MAXSEMS returns -1 (table full).
*   - SemFree(semaphore[105]) returns 0.
*   - Next SemCreate after free returns 0 and reuses the slot.
*   - "TEST PASSED" is printed on success.
*
* SYSTEM CALLS TESTED:
*   SemCreate (capacity limit), SemFree (slot reclamation), Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int semaphore[MAXSEMS + 1];
    int sem_result;
    int i;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    console_output(FALSE, "%s: Creating MAXSEMS semaphores\n", testName);
    for (i = 0; i < MAXSEMS; i++) {
        sem_result = SemCreate(0, &semaphore[i]);
        if (sem_result == -1)
            console_output(FALSE, "%s: SemCreate returned %2d at index %3d\n", testName, sem_result, i);
    }

    sem_result = SemCreate(0, &semaphore[MAXSEMS]);

    if (sem_result != -1)
        console_output(FALSE, "%s: Error - SemCreate returned %2d instead of -1\n", testName, sem_result);

    console_output(FALSE, "%s: Freeing one semaphore\n", testName);
    sem_result = SemFree(semaphore[105]);

    if (sem_result != 0)
        console_output(FALSE, "%s: Error - SemFree returned %2d instead of 0\n", testName, sem_result);

    sem_result = SemCreate(0, &semaphore[MAXSEMS]);

    if (sem_result != -1)
    if (sem_result == 0)
        console_output(FALSE, "%s: TEST PASSED - SemCreate returned %2d\n", testName, sem_result);
    else {
        console_output(FALSE, "%s: Error - SemCreate returned %2d instead of 0\n", testName, sem_result);
    }

    Exit(8);

    return 0;
}
