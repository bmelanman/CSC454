/** @file kproc.h
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-22-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef KPROC_H
# define KPROC_H

/* Includes */

# include "common.h"
# include "kmalloc.h"

/* Defines */

# define NO_THREAD 0 /* an always invalid thread id */

# define PROC_LIVE ( 0U )
# define PROC_TERM ( 1U )

# define TERM_OFFSET ( 8U )

/* Macros */

# define SET_TERM_STAT( a, b ) ( ( a ) << TERM_OFFSET | ( ( b ) & ( ( 1 << TERM_OFFSET ) - 1 ) ) )
# define GET_TERM_STAT( s )    ( ( s ) & ( ( 1 << TERM_OFFSET ) - 1 ) )

# define IS_PROC_TERMINATED( s ) ( ( ( s >> TERM_OFFSET ) & LWP_TERM ) == LWP_TERM )

/* Typedefs */

typedef void ( *kproc_t )( void * );

// rax, rbx, rcx, rdx, rdi, rsi
// r8, r9, r10, r11, r12, r13, r14, r15
// cs, ss, ds, es, fs, gs
// rbp, rsp, rip, rflags

typedef struct registers rfile;
struct __aligned( 16 ) __packed registers
{
    uint64_t rax; /* General purpose registers */
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t cs; /* Segment registers */
    uint64_t ss;
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
    uint64_t rbp;    /* Base pointer */
    uint64_t rsp;    /* Stack pointer */
    uint64_t rip;    /* Instruction pointer */
    uint64_t rflags; /* Flags register */
};

typedef struct threadinfo_st *kthread;
struct threadinfo_st
{
    pid_t pid;         /* Process id              */
    uint64_t *stack;   /* Base of allocated stack */
    size_t stacksize;  /* Size of allocated stack */
    rfile state;       /* saved registers         */
    uint32_t status;   /* exited? exit status?    */
    kthread lib_one;   /* Two pointers reserved   */
    kthread lib_two;   /* for use by the library  */
    kthread sched_one; /* Two more for            */
    kthread sched_two; /* schedulers to use       */
};

/* Tuple that describes a scheduler */
typedef struct scheduler
{
    void ( *init )( void );             /* initialize any structures     */
    void ( *shutdown )( void );         /* tear down any structures      */
    void ( *admit )( kthread new );     /* add a thread to the pool      */
    void ( *remove )( kthread victim ); /* remove a thread from the pool */
    kthread ( *next )( void );          /* select a thread to schedule   */
} *scheduler;

/* Global Variables */

/* Public Functions */

void rr_admit( kthread new_thread );
void rr_remove( kthread victim );
kthread rr_next( void );

kthread PROC_get_active_kthread( void );

/**
 * @brief This function is to be called in a loop at the end of kmain. It drives the entire
 * multi-tasking system, selecting the next thread to be ran. If no thread is able to run, then
 * `PROC_run()` will return.
 */
void PROC_run( void );

/**
 * @brief Adds a new thread to the multi-tasking system by allocating a new stack in the virtual
 * address space and initializing the thread's context such that the entry_point function gets
 * executed the next time this thread is scheduled. Note: This function does not actually schedule
 * the thread.
 * @param entry_point
 * @param arg
 */
kthread PROC_create_kthread( kproc_t entry_point, void *arg );

/**
 * @brief Selects the next thread to run, or the original thread that called PROC_run if no other
 * threads are available. Note: This function does not actually perform a context switch.
 */
void PROC_reschedule( void );

/**
 * @brief Invokes the scheduler and passes control from the current to the next eligible thread, or
 * returns to the current thread if no other eligble threads are available.
 */
// static inline void yield( void ) { asm volatile( "INT $123" ); }
void yield( void );

/**
 * @brief Exits the calling thread and destroys its state data, then calls the scheduler to pick a
 * new thread to run.
 */
void kexit( void );

#endif /* KPROC_H */

/*** End of File ***/