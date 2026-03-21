#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int SleepAndReport(void* strArgs);

/*********************************************************************************
*
* SystemCallsTest18
*
* PURPOSE:
*   Tests process timing using GetTimeofDay busy-wait delays of varying
*   durations. Three children each wait for a different number of seconds
*   (1, 2, and 3) using a spin loop on GetTimeofDay. Three children each sleep for a different number
*   of seconds (1, 2, and 3). The parent waits for all three and verifies they
*   complete in the correct order by observing the Wait return sequence.
*
* EXPECTED BEHAVIOR:
*   - Child sleeping 1 second should finish first.
*   - Child sleeping 2 seconds should finish second.
*   - Child sleeping 3 seconds should finish last.
*   - Each child reports elapsed wall-clock time.
*
* SYSTEM CALLS TESTED:
*   GetTimeofDay, Spawn, Wait, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int status = -1, pid, kidpid = -1;
    char nameBuffer[512];

    console_output(FALSE, "\n%s: started\n", testName);

    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Sleep1", testName);
    Spawn(nameBuffer, SleepAndReport, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned 1-second sleeper with PID %d\n", testName, pid);

    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Sleep2", testName);
    Spawn(nameBuffer, SleepAndReport, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned 2-second sleeper with PID %d\n", testName, pid);

    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Sleep3", testName);
    Spawn(nameBuffer, SleepAndReport, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
    console_output(FALSE, "%s: spawned 3-second sleeper with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d (expected 1-second sleeper)\n", testName, pid, status);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d (expected 2-second sleeper)\n", testName, pid, status);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for PID %d with status %d (expected 3-second sleeper)\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


/*
 * SleepAndReport
 *
 * Extracts the sleep duration from the last character of the process name,
 * sleeps for that many seconds, then reports elapsed time.  The exit status
 * matches the number of seconds slept so the parent can verify ordering.
 */
int SleepAndReport(void* strArgs)
{
    int startTime, endTime;
    int seconds;
    char* name = (char*)strArgs;

    /* Extract sleep duration from the last character of the name (e.g. "Sleep1" -> 1) */
    seconds = name[strlen(name) - 1] - '0';

    console_output(FALSE, "%s: started, will sleep %d second(s)\n", name, seconds);

    GetTimeofDay(&startTime);
    {
        int _bwNow;
        do { GetTimeofDay(&_bwNow); } while (_bwNow - startTime < seconds * 1000000);
    }
    GetTimeofDay(&endTime);

    console_output(FALSE, "%s: awake after %d microseconds (requested %d seconds)\n",
        name, endTime - startTime, seconds);

    Exit(seconds);

    return 0;
}