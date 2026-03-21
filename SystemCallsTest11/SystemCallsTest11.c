#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int SleepForTwoWithCPUTime(void* strArgs);

/*********************************************************************************
*
* SystemCallsTest11
*
* PURPOSE:
*   Tests the CPUTime system call, which returns the CPU time consumed by
*   the calling process (as opposed to wall-clock time from GetTimeofDay).
*   Three children each spin-wait until they have consumed 2000 milliseconds
*   of CPU time, then report the elapsed CPU time. The parent waits for all
*   three. This verifies that CPUTime correctly measures per-process CPU
*   consumption rather than global elapsed time.
*
* EXPECTED BEHAVIOR:
*   - Each child reports CPU time close to 2000 milliseconds.
*   - CPU time per process is independent of other concurrent processes.
*   - All three children complete and exit with status 9.
*   - All three Wait calls return successfully.
*   - Parent exits with status 8.
*
* SYSTEM CALLS TESTED:
*   CPUTime, Spawn, Wait, Exit
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int status = -1, pid, kidpid = -1;
    char nameBuffer[512];

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    /* Use the -Child naming convention for the child process name. */
    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child1", testName);

    Spawn(nameBuffer, SleepForTwoWithCPUTime, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);
    Spawn(nameBuffer, SleepForTwoWithCPUTime, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);
    Spawn(nameBuffer, SleepForTwoWithCPUTime, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);
    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);
    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}


int SleepForTwoWithCPUTime(void* strArgs)
{
    int startTime, time;

    console_output(FALSE, "%s: started\n", strArgs);

    CPUTime(&startTime);
    time = startTime;
    while ((time - startTime < 2000))
    {
        CPUTime(&time);
    }
    console_output(FALSE, "%s: Sleep complete - %d\n", strArgs, time - startTime);

    return 9;

}
