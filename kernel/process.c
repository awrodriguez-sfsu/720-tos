#include <kernel.h>
#include "../include/kernel.h"

PCB pcb[MAX_PROCS];


MEM_ADDR get_new_stack_frame(int number, void (*entry_point) (PROCESS, PARAM), PARAM param) {

    MEM_ADDR stack = (MEM_ADDR) 0xA0000 - (30720 * number);
    poke_l(stack     , param);               // param
    poke_l(stack -  4, (LONG) &pcb[number]); // self
    poke_l(stack -  8, (LONG) 0);            // return address (dummy value)
    poke_l(stack - 12, (LONG) entry_point);  // entry_point
    poke_l(stack - 16, (LONG) 0);            // %EAX register
    poke_l(stack - 20, (LONG) 2);            // %ECX register
    poke_l(stack - 24, (LONG) 3);            // %EDX register
    poke_l(stack - 28, (LONG) 4);            // %EBX register
    poke_l(stack - 32, (LONG) 5);            // %EBP register
    poke_l(stack - 36, (LONG) 6);            // %ESI register
    poke_l(stack - 40, (LONG) 7);            // %EDI register

    stack = stack - 40;

    return stack;
}

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM), int prio, PARAM param, char *name) {
    int i;
    for(i = 1; i < MAX_PROCS; i++) {
        if(pcb[i].used == FALSE) {
            pcb[i].magic = MAGIC_PCB;
            pcb[i].used = TRUE;
            pcb[i].priority = (unsigned short) prio;
            pcb[i].state = STATE_READY;
            pcb[i].esp = get_new_stack_frame(i, ptr_to_new_proc, param);
            pcb[i].first_port = NULL;
            pcb[i].name = name;

            add_ready_queue(&pcb[i]);
            return NULL;
        }
    }
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
        case 0:
            return "READY";
            break;
        case 1:
            return "SEND_BLOCKED";
            break;
        case 2:
            return "REPLY_BLOCKED";
            break;
        case 3:
            return "RECEIVE_BLOCKED";
            break;
        case 4:
            return "MESSAGE_BLOCKED";
            break;
        case 5:
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
    pcb->name           = "Boot process";

    int i;
    for(i = 1; i < MAX_PROCS; i++) {
        pcb[i].used = FALSE;
    }
}
