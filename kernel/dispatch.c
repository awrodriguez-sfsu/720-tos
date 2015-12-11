#include <kernel.h>
#include "disptable.c"
#include "../include/kernel.h"

PROCESS active_proc;

/*
 * Ready queues for all eight priorities.
 */
PCB* ready_queue [MAX_READY_QUEUES];

//int index = 0;

/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by proc is put in the ready queue.
 * The appropriate ready queue is determined by proc->priority.
 */
void add_ready_queue(PROCESS proc) {
    volatile int lock;
    DISABLE_INTR(lock);
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
    ENABLE_INTR(lock);
}

BOOL only_proc(PROCESS proc) {
    return (proc->next == proc) && (proc->prev == proc);
}

BOOL head_of_list(PROCESS proc) {
    return (ready_queue[proc->priority] == proc);
}

/* Debugging only */
//void print_ready_queue() {
//    clear_window(kernel_window);
//    int i;
//    for(i = 0; i < MAX_READY_QUEUES; i++) {
//        PROCESS priority_list = ready_queue[i];
//
//        if(priority_list != NULL) {
//            kprintf("-----priority %d-----\n", i);
//            PROCESS current = priority_list;
//            kprintf("%s next:%s prev:%s\n", current->name, current->next->name, current->prev->name);
//            while(current->next != priority_list) {
//                current = current->next;
//                kprintf("%s next:%s prev:%s\n", current->name, current->next->name, current->prev->name);
//            }
//        } else {
//            kprintf("priority %d is empty\n", i);
//        }
//    }
//}

void change_state(PROCESS proc, unsigned short state) {
    volatile int lock;
    DISABLE_INTR(lock);

    if(proc->state == STATE_READY && state != STATE_READY) {
        proc->state = state;
        remove_ready_queue(proc);
    } else if (proc->state != STATE_READY && state == STATE_READY) {
        proc->state = state;
        add_ready_queue(proc);
    } else {
        proc->state = state;
    }

    ENABLE_INTR(lock);
}

/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by proc is de-queued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc) {
    volatile int lock;
    DISABLE_INTR(lock);

//    print_ready_queue(); // Debugging

    PROCESS prev = proc->prev;
    PROCESS next = proc->next;

    next->prev = prev;
    prev->next = next;

    if(only_proc(proc)) {
        ready_queue[proc->priority] = NULL;
    } else if(head_of_list(proc)) {
        ready_queue[proc->priority] = next;
    }

    ENABLE_INTR(lock);
}

/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher() {
    volatile int lock;
    DISABLE_INTR(lock);
    int i;

    /* Check of there are processes with higher priorities */
    for (i = MAX_READY_QUEUES - 1; i > active_proc->priority; --i) {
        if(ready_queue[i] != NULL) {
            ENABLE_INTR(lock);
            return ready_queue[i];
        }
    }

    /* No processes with higher priorities existed */
    /* Pass to next process in same priority level even if it is self */
    if(ready_queue[active_proc->priority] != NULL) {
        ENABLE_INTR(lock);
        return active_proc->next;
    }

    /* No processes with higher priorities existed */
    /* No other process with same priority level */
    /* Check for processes with lower priority level */
    for(i = active_proc->priority - 1; i >= 0; --i) {
        if(ready_queue[i] != NULL) {
            ENABLE_INTR(lock);
            return ready_queue[i];
        }
    }

    ENABLE_INTR(lock);
    return NULL;
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

    /* pushing eflags and cs register */
    /* this allows the stack to look the same */
    /* between normal resign and interrupts */
    asm("pushfl");
    asm("cli");
    asm("popl %eax");
    asm("xchg (%esp), %eax");
    asm("pushl %cs");
    asm("pushl %eax");

    asm("pushl %eax");
    asm("pushl %ecx");
    asm("pushl %edx");
    asm("pushl %ebx");
    asm("pushl %ebp");
    asm("pushl %esi");
    asm("pushl %edi");

    asm("movl %%esp, %0" : "=r" (active_proc->esp) :);

    active_proc = dispatcher();

    asm("movl %0, %%esp" : : "r" (active_proc->esp));

    asm("popl %edi");
    asm("popl %esi");
    asm("popl %ebp");
    asm("popl %ebx");
    asm("popl %edx");
    asm("popl %ecx");
    asm("popl %eax");

    asm("iret");
}


/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher() {
    active_proc = NULL;

    int i;
    for(i = 0; i < MAX_READY_QUEUES; i++) {
        ready_queue[i] = NULL;
    }

    add_ready_queue(&pcb[0]);

    active_proc = ready_queue[pcb[0].priority];
}
