#include <kernel.h>
#include "../include/kernel.h"

PCB pcb[MAX_PROCS];


MEM_ADDR get_new_stack_frame(int number, void (*entry_point) (PROCESS, PARAM), PARAM param) {

    MEM_ADDR stack = (MEM_ADDR) 0xA0000 - (30720 * number);
    poke_l(stack     , param);               // param
    poke_l(stack -  4, (LONG) &pcb[number]); // self
    poke_l(stack -  8, (LONG) 0);            // return address (dummy value)
    if(interrupts_initialized) {
        poke_l(stack - 12, (LONG) 512);      // EFLAGS value
    } else {
        poke_l(stack - 12, (LONG) 0);        // EFLAGS value
    }
    poke_l(stack - 16, (LONG) 8);            // CS register
    poke_l(stack - 20, (LONG) entry_point);  // entry_point
    poke_l(stack - 24, (LONG) 0);            // %EAX register
    poke_l(stack - 28, (LONG) 0);            // %ECX register
    poke_l(stack - 32, (LONG) 0);            // %EDX register
    poke_l(stack - 36, (LONG) 0);            // %EBX register
    poke_l(stack - 40, (LONG) 0);            // %EBP register
    poke_l(stack - 44, (LONG) 0);            // %ESI register
    poke_l(stack - 48, (LONG) 0);            // %EDI register

    stack = stack - 48;

    return stack;
}

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM), int prio, PARAM param, char *name) {
    volatile int lock;
    DISABLE_INTR(lock);
    int i;
    for(i = 1; i < MAX_PROCS; i++) {
        if(pcb[i].used == FALSE) {
            pcb[i].magic = MAGIC_PCB;
            pcb[i].used = TRUE;
            pcb[i].priority = (unsigned short) prio;
            pcb[i].state = STATE_READY;
            pcb[i].esp = get_new_stack_frame(i, ptr_to_new_proc, param);
            pcb[i].first_port = create_new_port(&pcb[i]);
            pcb[i].name = name;

            add_ready_queue(&pcb[i]);
            ENABLE_INTR(lock);
            return pcb[i].first_port;
        }
    }
    ENABLE_INTR(lock);
    return (PORT) NULL;
}

PROCESS fork() {
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}

BOOL is_active_proc(PROCESS proc) {
    if(active_proc == proc) {
        return '*';
    } else {
        return ' ';
    }
}

char* get_state_name(unsigned short state) {
    switch(state) {
        case STATE_READY:
            return "READY";
            break;
        case STATE_SEND_BLOCKED:
            return "SEND_BLOCKED";
            break;
        case STATE_REPLY_BLOCKED:
            return "REPLY_BLOCKED";
            break;
        case STATE_RECEIVE_BLOCKED:
            return "RECEIVE_BLOCKED";
            break;
        case STATE_MESSAGE_BLOCKED:
            return "MESSAGE_BLOCKED";
            break;
        case STATE_INTR_BLOCKED:
            return "INTR_BLOCKED";
            break;
        default:
            return "";
    }
}

void print_process(WINDOW* wnd, PROCESS p) {
    wprintf(wnd, "%-24s   %c       %-4d  %-8s\n", get_state_name(p->state), is_active_proc(p), p->priority, p->name);
}

void print_all_processes(WINDOW* wnd) {
    wprintf(wnd, "State                    Active Priority Name\n");
    wprintf(wnd, "---------------------------------------------\n");
    int i;
    for(i = 0; i < MAX_PROCS; i++) {
        if(pcb[i].used) {
            print_process(wnd, &pcb[i]);
        }
    }
}

void init_process() {
    pcb[0].magic        = MAGIC_PCB;
    pcb[0].used         = TRUE;
    pcb[0].state        = STATE_READY;
    pcb[0].priority     = 1;
    pcb[0].first_port   = NULL;
    pcb[0].name         = "Boot process";

    int i;
    for(i = 1; i < MAX_PROCS; i++) {
        pcb[i].used = FALSE;
    }
}
