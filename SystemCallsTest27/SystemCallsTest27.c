
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int ReportAndVerifyPid(char* strArgs);

/*********************************************************************************
*
* SystemCallsTest27
*
* PURPOSE:
*   Tests that GetPID returns the correct PID for each process, and that the
*   PID a child reports via GetPID matches the PID returned to the parent by
*   Spawn. This cross-verification catches bugs where GetPID returns a stale,
*   wrong, or shared value rather than the calling process's own PID.
*
*   Each child calls GetPID and exits with its PID as the exit status. The
*   parent compares the status returned by Wait against the PID that Spawn
*   originally returned for that child. If they match, the PID is correct.
*   This avoids any struct-passing race conditions by using only the exit
*   status as the communication channel between child and parent.
*
* EXPECTED BEHAVIOR:
*   - GetPID inside each child returns the same PID that Spawn returned.
*   - All four children exit with their own PID as status.
*   - Wait returns status == pid for each child.
*   - All four report PASS.
*
* SYSTEM CALLS TESTED:
*   GetPID, Spawn, Wait, Exit
*
*********************************************************************************/

int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int spawnedPids[4];
    int pid;
    int status;
    int i;

    console_output(FALSE, "\n%s: started\n", testName);

    /* Spawn four children -- each will exit with its own PID as status */
    for (i = 0; i < 4; i++) {
        snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child%d", testName, i + 1);
        Spawn(nameBuffer, ReportAndVerifyPid, nameBuffer,
            THREADS_MIN_STACK_SIZE, 3, &pid);
        spawnedPids[i] = pid;
        console_output(FALSE, "%s: spawned child %d with PID %d\n",
            testName, i + 1, pid);
    }

    /* Wait for all four -- status should equal the PID Spawn returned */
    for (i = 0; i < 4; i++) {
        Wait(&pid, &status);

        /* Find which spawn this PID corresponds to */
        int j, found = 0;
        for (j = 0; j < 4; j++) {
            if (spawnedPids[j] == pid) {
                found = 1;
                if (status == pid) {
                    console_output(FALSE, "%s: PID %d GetPID matched PASS\n",
                        testName, pid);
                }
                else {
                    console_output(FALSE, "%s: PID %d GetPID returned %d FAIL\n",
                        testName, pid, status);
                }
                break;
            }
        }
        if (!found) {
            console_output(FALSE, "%s: Wait returned unknown PID %d FAIL\n",
                testName, pid);
        }
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


/*
 * ReportAndVerifyPid
 *
 * Calls GetPID and exits with the returned PID as the exit status.
 * The parent compares this against what Spawn returned to verify correctness.
 */
int ReportAndVerifyPid(char* strArgs)
{
    int myPid;

    GetPID(&myPid);
    console_output(FALSE, "%s: GetPID returned %d\n", strArgs, myPid);

    Exit(myPid);

    return 0;
}