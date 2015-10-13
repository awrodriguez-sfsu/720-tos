#include <kernel.h>
#include "disptable.c"
#include "../include/kernel.h"

PROCESS active_proc;

/*
 * Ready queues for all eight priorities.
 */
PCB* ready_queue [MAX_READY_QUEUES];


/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by proc is put in the ready queue.
 * The appropriate ready queue is determined by proc->priority.
 */
void add_ready_queue(PROCESS proc) {
    /* empty queue */
    if(ready_queue[proc->priority] == NULL) {
        proc->next = proc;
        proc->prev = proc;
        ready_queue[proc->priority] = proc;
    } else {
        PCB* first = ready_queue[proc->priority];
        PCB* last = first->prev;

        first->prev = proc;
        last->next = proc;
        proc->prev = last;
        proc->next = first;
    }
}

BOOL only_proc(PROCESS proc) {
    return (proc->next == proc->prev);
}

BOOL head_of_list(PROCESS proc) {
    return (ready_queue[proc->priority] == proc);
}

/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by proc is de-queued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc) {

    PCB* prev = proc->prev;
    PCB* next = proc->next;

    next->prev = prev;
    prev->next = next;
    proc->next = NULL;
    proc->prev = NULL;

    if(only_proc(proc)) {
        ready_queue[proc->priority] = NULL;
    } else if(head_of_list(proc)) {
        ready_queue[proc->priority] = next;
    }
}


/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher() {

}


/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign() {

}


/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher() {
    ready_queue[0] = NULL;
    int i;
    for(i = 2; i < MAX_READY_QUEUES; i++) {
        ready_queue[i] = NULL;
    }
}
