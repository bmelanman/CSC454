/** @file kproc.c
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-22-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "kproc.h"

/* Private Includes */

#include "idt.h"

/* Private Defines and Macros */

#define PROC_STACK_SIZE ( 0x1000U )

/* Private Types and Enums */

/* Global Variables */

// Main system thread
kthread main_kthread = NULL;

// Default round-robin scheduler
struct scheduler rr_sched_struct = { NULL, NULL, rr_admit, rr_remove, rr_next };
scheduler rr_scheduler = &rr_sched_struct;

// Current thread and scheduler
kthread curr_kthread = NULL, next_kthread = NULL;
scheduler active_sched = NULL;

// Counter and array for thread IDs
pid_t pid_cnt = NO_THREAD;
// kthread *kthread_list = NULL;

// Flag to keep track of setup and teardown
bool setup = false;

/* Private Functions */

/**
 * @brief Changes the current scheduler to the given one. If the given scheduler is NULL, a
 * round-robin scheduler is used instead. All threads are transferred from the old scheduler to the
 * new one in next() order.
 * @param s The new scheduler
 */
void PROC_set_scheduler( scheduler s )
{
    // Check for valid input
    if ( active_sched == NULL || s == NULL )
    {
        // If `s`s is NULL, set the scheduler to the default round robin
        // scheduler, otherwise just use `s`
        active_sched = ( s == NULL ? rr_scheduler : s );

        // Run init
        if ( active_sched->init != NULL )
        {
            active_sched->init();
        }

        return;
    }

    // Add the current thread to the new scheduler first
    if ( curr_kthread != NULL )
    {
        s->admit( curr_kthread );
    }

    // Get the next thread from the old scheduler
    kthread th = active_sched->next();

    // Transfer the threads to the new scheduler
    while ( th != NULL )
    {
        // Add the thread to the new scheduler
        s->admit( th );
        // Remove the thread from the old schedule
        active_sched->remove( th );

        th = active_sched->next();
    }

    // Shutdown the old scheduler
    if ( active_sched->shutdown != NULL )
    {
        active_sched->shutdown();
    }

    // Set the new scheduler
    active_sched = s;

    // Init the new scheduler
    if ( active_sched->init != NULL )
    {
        active_sched->init();
    }
}

void PROC_init( void )
{
    if ( setup == true )
    {
        return;
    }

    // Initialize the main thread
    main_kthread = (kthread)kmalloc( sizeof( kthread ) );

    main_kthread->pid = 0;
    main_kthread->status = SET_TERM_STAT( 0, PROC_LIVE );
    main_kthread->stack = NULL;  // The main thread runs on the kernel stack
    main_kthread->stacksize = 0;

    // Set the current thread to the main thread
    curr_kthread = main_kthread;

    // Initialize the thread list
    // kthread_list = (kthread *)kcalloc( 7, sizeof( kthread ) );

    // Initialize the scheduler
    if ( active_sched == NULL )
    {
        // Use the default scheduler
        PROC_set_scheduler( NULL );
    }

    // Setup the yield trap in the IDT
    // idt_set_descriptor( 0x80, yield, ( PRESENT_FLAG | INTERRUPT_GATE_FLAG ) );

    // Set the setup flag
    setup = true;
}

/* Public Functions */

// DEBUG: This function is only for debugging purposes
kthread PROC_get_active_kthread( void ) { return curr_kthread; }

// Called in a loop at the end of kmain. This drives the entire multi-tasking system. The next
// thread gets selected and run. Threads can yield, exit, etc. If no thread is able to run then
// PROC_run() returns.
void PROC_run( void )
{
    // Initialize the multi-tasking system
    PROC_init();

    // Select the next thread to run
    curr_kthread = active_sched->next();

    // If no thread is able to run then PROC_run() returns
    if ( curr_kthread == NULL ) return;

    // Run the next thread
    curr_kthread->status = SET_TERM_STAT( 0, PROC_LIVE );
    active_sched->admit( curr_kthread );
}

// Adds a new thread to the multi-tasking system. This requires allocating a new stack in the
// virtual address space and initializing the thread's context such that the entry_point function
// gets executed the next time this thread is scheduled.
// NOTE: This function does not actually schedule the thread.
kthread PROC_create_kthread( kproc_t entry_point, void *arg )
{
    kthread new_kthread = (kthread)kmalloc( sizeof( kthread ) );

    // Initialize the new thread
    new_kthread->pid = ++pid_cnt;
    new_kthread->status = SET_TERM_STAT( 0, PROC_LIVE );

    // Allocate a new stack in the virtual address space
    new_kthread->stack = (uint64_t *)kmalloc( PROC_STACK_SIZE );
    new_kthread->stacksize = PROC_STACK_SIZE;

    // Initialize the thread's context
    new_kthread->state.rsp = (uint64_t)new_kthread->stack;
    new_kthread->state.rbp = (uint64_t)new_kthread->stack;

    // The entry_point function gets executed the next time this thread is scheduled
    // new_kthread->state.rip = (uint64_t)entry_point;
    new_kthread->state.rdi = (uint64_t)arg;

    // Push the address of the entry_point function onto the thread's stack
    // new_kthread->state.rsp -= sizeof( uint64_t );
    // *(uint64_t *)new_kthread->state.rsp = (uint64_t)entry_point;
    new_kthread->stack[1] = (uint64_t)entry_point;

    // Push the address of the kexit function onto the thread's stack
    // new_kthread->state.rsp -= sizeof( uint64_t );
    // *(uint64_t *)new_kthread->state.rsp = (uint64_t)kexit;

    return new_kthread;
}

// Selects the next thread to run. It must select the "thread" that called PROC_run if not other
// threads are available to run. This function does not actually perform a context switch.
void PROC_reschedule( void )
{
    // Select the next thread to run
    curr_kthread = active_sched->next();

    // If no thread is able to run then PROC_run() returns
    if ( curr_kthread == NULL ) return;
}

// Invokes the scheduler and passes control to the next eligible thread. It is possible to return to
// the thread that alled yield if no other eligible threads exist. To make context switch
// implementation more consistent, and your life slightly easier, I suggest making yield() a trap
// that calls the actual yield implementation. This way the stack frame is always the same and you
// can reuse the assembly interrupt handler you have already written.
// TODO: Implement yield() as a trap that performs a context swap
void yield( void )
{
    // Select the next thread to run
    curr_kthread = active_sched->next();

    // If no thread is able to run then PROC_run() returns
    if ( curr_kthread == NULL ) return;
}

// Exits and destroys all the state of the thread that calls kexit. Needs to run the scheduler to
// pick another process. I also suggest you use a trap-based implementation AND the IST mechanism so
// that the trap handler runs on a different stack. Running on a different stack makes it possible
// to free the thread's stack without pulling the rug out from under yourself.
void kexit( void )
{
    // Free the thread's stack
    kfree( curr_kthread->stack );

    // Free the thread's state
    kfree( curr_kthread );

    // Select the next thread to run
    curr_kthread = active_sched->next();

    // If no thread is able to run then PROC_run() returns
    if ( curr_kthread == NULL ) return;
}

/*** End of File ***/