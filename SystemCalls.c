#define _CRT_SECURE_NO_WARNINGS
#define SYSTEM_CALLS_PROJECT

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <THREADSLib.h>
#include <Messaging.h>
#include <Scheduler.h>
#include <TList.h>
#include <SystemCalls.h>
#include "libuser.h"

/*********************************************************************************
*
* SystemCalls.c
*
* PURPOSE:
*   Implements the System Calls layer of the THREADS operating system kernel.
*   This file establishes the boundary between user-mode and kernel-mode
*   processes. User-level processes access kernel functionality exclusively
*   through system calls, which trap the kernel via the
*   THREADS_SYS_CALL_INTERRUPT mechanism.
*
*   This file must implement:
*     - Initialization of the system call vector table
*     - A system call handler for all supported system calls
*     - Kernel-level implementations: sys_spawn, sys_wait, sys_exit
*     - Semaphore kernel functions: k_semcreate, k_semp, k_semv, k_semfree
*     - User process launch and lifecycle management
*
*   See the System Calls Project Definition document for full specifications.
*
*********************************************************************************/

/* -------------------- Typedefs and Structs -------------------------------- */

/*****************************************************************************
*  checkKernelMode
*  Checks the PSR for kernel mode and halts if in user mode.
*****************************************************************************/
struct psr_bits {
    unsigned int cur_int_enable : 1;
    unsigned int cur_mode : 1;
    unsigned int prev_int_enable : 1;
    unsigned int prev_mode : 1;
    unsigned int unused : 28;
};

union psr_values {
    struct psr_bits  bits;
    unsigned int     integer_part;
};

static inline void checkKernelMode(const char* functionName)
{
    union psr_values psrValue;
    psrValue.integer_part = get_psr();
    if (psrValue.bits.cur_mode == 0)
    {
        console_output(FALSE, "Kernel mode expected, but function called in user mode.\n");
        stop(1);
    }
}

/*
 * SemData
 *
 * Kernel-side state for a single semaphore. Add members as needed.
 */
typedef struct sem_data
{
    int status;     /* 0 = free, 1 = in use */
    int sem_id;     /* unique identifier returned to user */
    int count;      /* current semaphore value */

    /* Add additional members needed. */
} SemData;


/*
 * UserProcess
 *
 * Kernel-side state for a single user-level process.
 * Add members as needed.
 */
typedef struct user_proc
{
    struct user_proc* pNextSibling;
    struct user_proc* pPrevSibling;
    struct user_proc* pNextSem;
    struct user_proc* pPrevSem;
    TList             children;
    int               mboxStartup;
    int              (*startFunc)(char*);   /* entry point for this process */

    /* Add additional members needed. */
} UserProcess;


/* -------------------- Constants ------------------------------------------ */

#define MAXSEMS     200
#define USERMODE    set_psr(get_psr() & ~PSR_KERNEL_MODE)


/* -------------------- Globals -------------------------------------------- */

static UserProcess  userProcTable[MAXPROC];
static SemData      semTable[MAXSEMS];


/* -------------------- Prototypes ----------------------------------------- */

void    sys_exit(int resultCode);
int     sys_wait(int* pStatus);
int     sys_spawn(char* name, int (*startFunc)(char*), char* arg,
    int stackSize, int priority);

int     k_semcreate(int initial_value);
int     k_semp(int sem_id);
int     k_semv(int sem_id);
int     k_semfree(int sem_id);

static int  launchUserProcess(char* pArg);
static void system_call_handler(system_call_arguments_t* args);
static void sysNull(system_call_arguments_t* args);

int MessagingEntryPoint(char*);
extern int SystemCallsEntryPoint(char*);


/*********************************************************************************
*
* MessagingEntryPoint
*
* PURPOSE:
*   Kernel entry point called by the THREADS framework on startup. Initializes
*   all system call infrastructure and launches the first user-level process.
*
*********************************************************************************/
int MessagingEntryPoint(char* arg)
{
    int status = -1;
    int pid;

    checkKernelMode(__func__);

    /* Initialize the semaphore table */

    /* Initialize the system call vector */
    for (int i = 0; i < THREADS_MAX_SYSCALLS; i++)
    {
        systemCallVector[i] = system_call_handler;
    }

    /* Initialize the user process table */
    for (int i = 0; i < MAXPROC; ++i)
    {
        userProcTable[i].mboxStartup = mailbox_create(1, sizeof(int));

        /* Add additional initialization as needed. */
    }

    pid = sys_spawn("SystemCalls", SystemCallsEntryPoint, NULL,
        THREADS_MIN_STACK_SIZE * 4, 3);

    sys_wait(&status);

    return 0;

} /* MessagingEntryPoint */


/*********************************************************************************
*
* launchUserProcess
*
* PURPOSE:
*   Kernel-mode trampoline that every user process runs before switching to
*   user mode. Waits for sys_spawn to finish initializing the process table
*   entry, then drops to user mode and calls the process's entry point function.
*   If the process is signaled before it starts, it exits immediately.
*
*   NOTE: This function runs in kernel mode up until USERMODE is called.
*   Everything after that executes in user mode.
*
*********************************************************************************/
static int launchUserProcess(char* pArg)
{
    UserProcess* pProc;
    int resultCode;

    /* Wait for sys_spawn to finish initializing our process table entry */
    mailbox_receive(userProcTable[k_getpid() % MAXPROC].mboxStartup, NULL, 0, TRUE);

    if (signaled())
    {
        console_output(FALSE, "%s - Process signaled in launch.\n", "launchUserProcess");
        sys_exit(0);
    }

    USERMODE;

    pProc = &userProcTable[k_getpid() % MAXPROC];
    resultCode = pProc->startFunc(NULL);    /* pass correct arg */

    /* If the entry point returns rather than calling Exit, treat as Exit */
    sys_exit(resultCode);

    return 0;

} /* launchUserProcess */


/*********************************************************************************
*
* sys_spawn
*
* PURPOSE:
*   Kernel-level implementation of the Spawn system call. Creates a new
*   user-level process, initializes its process table entry, and signals
*   it to begin execution.
*
* PARAMETERS:
*   name      - Name string for the new process
*   startFunc - Entry point function for the new process
*   arg       - Argument string passed to the entry point
*   stackSize - Stack size for the new process
*   priority  - Scheduling priority for the new process
*
* RETURNS:
*   PID of the new process on success, -1 on failure.
*
*********************************************************************************/
int sys_spawn(char* name, int (*startFunc)(char*), char* arg,
    int stackSize, int priority)
{
    int pid = -1;
    UserProcess* pProcess;

    /* Validate parameters */

    pid = k_spawn(name, launchUserProcess, arg, stackSize, priority);
    if (pid >= 0)
    {
        pProcess = &userProcTable[pid % MAXPROC];
        pProcess->startFunc = startFunc;

        /* Signal the new process to begin */
        mailbox_send(pProcess->mboxStartup, NULL, 0, FALSE);
    }

    return pid;

} /* sys_spawn */


/*********************************************************************************
*
* sys_wait
*
* PURPOSE:
*   Blocks the calling process until one of its children exits. Returns that
*   child's PID and exit status. Returns an error if no children exist.
*
* PARAMETERS:
*   pStatus - Output: receives the child's exit status.
*
* RETURNS:
*   PID of the child that exited, or -1 if no children exist.
*
*********************************************************************************/
int sys_wait(int* pStatus)
{
    return -1;

} /* sys_wait */


/*********************************************************************************
*
* sys_exit
*
* PURPOSE:
*   Terminates the calling user-level process with the given status code.
*   Notifies the parent so its Wait can be satisfied. Cleans up process
*   table state.
*
*   A process must not call Exit while it has active children.
*
* PARAMETERS:
*   resultCode - Exit status returned to the waiting parent.
*
*********************************************************************************/
void sys_exit(int resultCode)
{

} /* sys_exit */


/*********************************************************************************
*
* k_semcreate
*
* PURPOSE:
*   Creates and initializes a semaphore with the given initial value.
*
* RETURNS:
*   Semaphore identifier (>= 0) on success, -1 on failure.
*
*********************************************************************************/
int k_semcreate(int initial_value)
{
    int sem_id = -1;

    return sem_id;

} /* k_semcreate */


/*********************************************************************************
*
* k_semp
*
* PURPOSE:
*   Decrements the semaphore. Blocks if value is zero. If freed while blocked,
*   the process must exit with code 1.
*
* RETURNS:
*   0 on success, -1 if sem_id is invalid.
*
*********************************************************************************/
int k_semp(int sem_id)
{
    int result = -1;

    return result;

} /* k_semp */


/*********************************************************************************
*
* k_semv
*
* PURPOSE:
*   Increments the semaphore. Unblocks one waiting process if any exist.
*
* RETURNS:
*   0 on success, -1 if sem_id is invalid.
*
*********************************************************************************/
int k_semv(int sem_id)
{
    int result = -1;

    return result;

} /* k_semv */


/*********************************************************************************
*
* k_semfree
*
* PURPOSE:
*   Frees the semaphore. All blocked processes must exit with code 1.
*
* RETURNS:
*   0  - freed, no waiters.
*   1  - freed, waiters were present.
*   -1 - invalid or not in use.
*
*********************************************************************************/
int k_semfree(int sem_id)
{
    int result = -1;

    return result;

} /* k_semfree */


/*********************************************************************************
*
* sysNull
*
* PURPOSE:
*   Error handler for unrecognized system call IDs. Terminates the calling
*   process.
*
*********************************************************************************/
static void sysNull(system_call_arguments_t* args)
{
    console_output(FALSE, "nullsys(): Invalid syscall %d. Halting...\n", args->call_id);
    sys_exit(1);

} /* sysNull */


/*********************************************************************************
*
* system_call_handler
*
* PURPOSE:
*   Single dispatch handler registered for all system call vector entries.
*   Extracts parameters from args->arguments[], calls the appropriate kernel
*   function, and writes return values back before restoring user mode.
*
*   Argument mappings for each system call are defined in the spec document.
*
*   IMPORTANT: Always restore user mode (USERMODE) before returning, unless
*   the call does not return (e.g. SYS_EXIT).
*
*   If signaled while in the handler, exit immediately.
*
* The system_call_arguments_t structure:
*   args->call_id        - identifies which system call was invoked
*   args->arguments[0-4] - input and/or output parameters (see spec)
*
*********************************************************************************/
static void system_call_handler(system_call_arguments_t* args)
{
    if (args == NULL)
    {
        console_output(FALSE, "system_call_handler(): NULL args, terminating.\n");
        sys_exit(1);
    }

    switch (args->call_id)
    {
    case SYS_SPAWN:
        /*
         * Argument mapping (SYS_SPAWN):
         *   INPUT:  args->arguments[4] = name (char*)
         *           args->arguments[0] = func (int(*)(char*))
         *           args->arguments[1] = arg  (char*)
         *           args->arguments[2] = stack_size (int)
         *           args->arguments[3] = priority (int)
         *   OUTPUT: args->arguments[0] = pid
         *           args->arguments[3] = return value (0 on success, -1 on failure)
         */
        args->arguments[0] = (intptr_t)sys_spawn(
            (char*)args->arguments[4],          /* name      */
            (int (*)(char*))args->arguments[0], /* func      */
            (char*)args->arguments[1],          /* arg       */
            (int)args->arguments[2],            /* stackSize */
            (int)args->arguments[3]);           /* priority  */
        args->arguments[3] = (intptr_t)((intptr_t)args->arguments[0] >= 0 ? 0 : -1);
        break;

        /* Add remaining cases here */

    default:
        sysNull(args);
        break;
    }

    if (signaled())
    {
        console_output(FALSE, "Process signaled while in system call.\n");
        sys_exit(0);
    }

    USERMODE;

} /* system_call_handler */