/** @file sched.c
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

/* Private Defines and Macros */
#define sched_next sched_one
#define sched_prev sched_two

/* Global Variables */

// Linked list of threads
kthread ll_head = NULL, ll_tail = NULL;
size_t ll_len = 0;

/* Helper Functions */

#define ASSERT_FALSE_MSG( condition, message, ... ) \
    if ( condition ) OS_ERROR_HALT( message, ##__VA_ARGS__ )

// Throw an error when an invalid configuration occurs
void validate_ll( void )
{
    ASSERT_FALSE_MSG(
        ( ll_head == NULL && ll_tail != NULL ),
        "Linked list head is NULL but tail is not! ll_len = %lu\n", ll_len
    );

    ASSERT_FALSE_MSG(
        ( ll_head != NULL && ll_tail == NULL ),
        "Linked list tail is NULL but head is not! ll_len = %lu\n", ll_len
    );

    ASSERT_FALSE_MSG(
        ( ll_head == NULL && ll_tail == NULL && ll_len != 0 ),
        "NULL linked list has a length greater than 0! ll_len = %lu\n", ll_len
    );

    ASSERT_FALSE_MSG(
        ( ll_head != NULL && ll_tail != NULL && ll_len == 0 ),
        "Non-NULL linked list has a length of 0! ll_len = %lu\n", ll_len
    );

    ASSERT_FALSE_MSG(
        ( ll_head == ll_tail && ll_head != NULL && ll_len != 1 ),
        "Linked list containing a single node does not have a length of 1!  \n"
        "  ll_len = %lu                                                     \n"
        "                                                                   \n"
        "Note:                                                              \n"
        " ll_head->sched_next %s NULL                                       \n"
        " ll_head->sched_prev %s NULL                                       \n",
        ll_len, ( ll_head->sched_next == NULL ? "==" : "!=" ),
        ( ll_head->sched_prev == NULL ? "==" : "!=" )
    );
}

/* Round Robin Scheduler Functions */

// Add a thread to the pool
void rr_admit( kthread new_thread )
{
    // Error checking
    validate_ll();

    if ( ll_len == 0 )
    {
        ll_head = new_thread;
        ll_tail = new_thread;

        // Make sure there are no erroneous connections
        new_thread->sched_next = NULL;
        new_thread->sched_prev = NULL;
    }
    else
    {
        // Add the thread to the tail of the linked list
        ll_tail->sched_next = new_thread;
        new_thread->sched_prev = ll_tail;
        ll_tail = new_thread;
    }

    // Increment the linked list size
    ll_len++;
}

// Remove a thread from the pool
void rr_remove( kthread victim )
{
    // Error checking
    validate_ll();

    // Is the list empty?
    if ( ll_len == 0 )
    {
        return;
    }

    // Is the thread the head AND the tail?
    if ( ll_len == 1 )
    {
        ll_head = NULL;
        ll_tail = NULL;
    }
    // Is the thread just the head?
    else if ( victim == ll_head )
    {
        ll_head = victim->sched_next;
        ll_head->sched_prev = NULL;
    }
    // Is the thread just the tail?
    else if ( victim == ll_tail )
    {
        ll_tail = ll_tail->sched_prev;
        ll_tail->sched_next = NULL;
    }
    // Otherwise, just cut it out
    else
    {
        victim->sched_prev->sched_next = victim->sched_next;
        victim->sched_next->sched_prev = victim->sched_prev;
    }

    // Remove the old connections
    victim->sched_next = NULL;
    victim->sched_prev = NULL;

    // Decrement the length
    ll_len--;
}

// Select the next thread to schedule
kthread rr_next( void )
{
    validate_ll();

    // Is the list empty?
    if ( ll_len == 0 )
    {
        return NULL;
    }
    // Is there only one item?
    if ( ll_len == 1 )
    {
        return ll_head;
    }

    // The list head will be served and moved to the back of the line
    kthread now_serving = ll_head;

    // Move the head to the next thread
    ll_head = ll_head->sched_next;
    ll_head->sched_prev = NULL;

    // Move the old head to the tail
    ll_tail->sched_next = now_serving;
    now_serving->sched_prev = ll_tail;
    now_serving->sched_next = NULL;
    ll_tail = now_serving;

    // Return the element that was moved to the end of the linked list
    return now_serving;
}

/*** End of File ***/