#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*********************************************************************************
*
* SystemCallsTest24
*
* PURPOSE:
*   Tests that a semaphore created with an initial value of N allows exactly N
*   successful SemP operations before blocking. This verifies the correctness of
*   the initial value parameter to SemCreate and the counting behavior of the
*   semaphore.
*
*   The test creates a semaphore with initial value 3. It spawns one child that
*   performs 5 SemP operations. The first 3 should succeed without blocking.
*   The 4th should block. A second child then performs 2 SemV operations to
*   release the blocked child. The child exits after all 5 SemPs complete.
*
*   Additionally, the test verifies that a semaphore created with initial value 0
*   blocks immediately on the first SemP.
*
* EXPECTED BEHAVIOR:
*   - SemCreate(3) allows 3 immediate SemPs.
*   - 4th SemP blocks until SemV is called.
*   - SemCreate(0) blocks on the first SemP.
*   - All SemP return values are 0 on success.
*
* SYSTEM CALLS TESTED:
*   SemCreate (with initial value > 0), SemP, SemV, Spawn, Wait, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    int semaphore;
    int sem_result;
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    int status;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);

    /* Create semaphore with initial value 3 */
    sem_result = SemCreate(3, &semaphore);
    console_output(FALSE, "%s: SemCreate(3) returned %d, semaphore = %d\n",
                   testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* Child 1: performs 5 SemPs - first 3 succeed, 4th blocks, 5th after SemVs */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 5, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: spawned 5-SemP child with PID %d\n", testName, pid);

    /* Child 2: performs 2 SemVs to unblock child 1 (which needs 2 more after initial 3) */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 2, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned 2-SemV child with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d\n", testName, pid, status);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d\n", testName, pid, status);

    /* Now test SemCreate with initial value 0 blocks immediately */
    console_output(FALSE, "%s: Testing SemCreate(0) blocks on first SemP\n", testName);

    SemFree(semaphore);

    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: SemCreate(0) returned %d, semaphore = %d\n",
                   testName, sem_result, semaphore);

    /* Child 3: 1 SemP (should block immediately) */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: spawned 1-SemP child with PID %d (should block)\n", testName, pid);

    /* Child 4: 1 SemV to release child 3 */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 1, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned 1-SemV child with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d\n", testName, pid, status);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}
