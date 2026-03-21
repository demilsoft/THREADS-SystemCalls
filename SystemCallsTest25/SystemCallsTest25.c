#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int ExitWithKnownStatus(char* strArgs);

/*********************************************************************************
*
* SystemCallsTest25
*
* PURPOSE:
*   Tests that Wait correctly receives the exit status from the child process
*   that actually exited, and that the status value is preserved exactly.
*   This is a cross-verification test: each child is spawned with a unique
*   known exit status, and the parent verifies that what Wait returns matches
*   what that specific child passed to Exit.
*
*   This matters because a naive implementation might mix up statuses when
*   multiple children exit in rapid or unpredictable order. The test spawns
*   five children each exiting with a distinct prime number status so that
*   any mix-up is immediately obvious in the output.
*
*   The exit status is encoded into the process name string using a ':'
*   separator (e.g. "SystemCallsTest25-Child1:11") and parsed by the child,
*   following the same convention used by CreateSysCallsTestArgs/StartDoSemsAndExit.
*
*   The test also verifies that the pid output from Wait matches the child
*   that actually exited, providing a complete pid-to-status mapping check.
*
* EXPECTED BEHAVIOR:
*   - Five children exit with statuses 11, 13, 17, 19, 23 respectively.
*   - Each Wait returns one (pid, status) pair.
*   - The status returned for each pid matches the known status for that child.
*   - No status is duplicated or lost.
*
* SYSTEM CALLS TESTED:
*   Spawn, Wait, Exit (status propagation)
*
*********************************************************************************/

/* Table to record which PID was spawned with which expected status */
static int spawnedPid[5];
static int expectedStatus[5] = { 11, 13, 17, 19, 23 };

int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int pid;
    int status;
    int i;

    console_output(FALSE, "\n%s: started\n", testName);

    /* Spawn five children. The exit status is encoded after ':' in the name
       so the child receives a valid char* and can parse the value out. */
    for (i = 0; i < 5; i++) {
        snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child%d:%d",
            testName, i + 1, expectedStatus[i]);
        Spawn(nameBuffer, ExitWithKnownStatus, nameBuffer,
            THREADS_MIN_STACK_SIZE, 3, &pid);
        spawnedPid[i] = pid;
        console_output(FALSE, "%s: spawned child %d with PID %d, expected exit status %d\n",
            testName, i + 1, pid, expectedStatus[i]);
    }

    /* Wait for all five and verify each status matches the known value for that PID */
    for (i = 0; i < 5; i++) {
        int j;
        int found = 0;

        Wait(&pid, &status);

        for (j = 0; j < 5; j++) {
            if (spawnedPid[j] == pid) {
                found = 1;
                if (status == expectedStatus[j]) {
                    console_output(FALSE, "%s: PID %d exited with status %d PASS\n",
                        testName, pid, status);
                }
                else {
                    console_output(FALSE, "%s: PID %d exited with status %d but expected %d FAIL\n",
                        testName, pid, status, expectedStatus[j]);
                }
                break;
            }
        }
        if (!found) {
            console_output(FALSE, "%s: Wait returned unknown PID %d with status %d FAIL\n",
                testName, pid, status);
        }
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


/*
 * ExitWithKnownStatus
 *
 * Parses the exit status from the name string after the ':' separator
 * (e.g. "SystemCallsTest25-Child1:11" -> exits with 11).
 * Uses the same convention as StartDoSemsAndExit in TestCommon.c.
 */
int ExitWithKnownStatus(char* strArgs)
{
    int exitStatus = 0;
    char* separator = strchr(strArgs, ':');

    if (separator != NULL) {
        separator[0] = '\0';
        separator++;
        sscanf(separator, "%d", &exitStatus);
    }

    console_output(FALSE, "%s: exiting with status %d\n", strArgs, exitStatus);
    Exit(exitStatus);

    return 0;
}