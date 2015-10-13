#include <kernel.h>
#include "../include/kernel.h"

PCB pcb[MAX_PROCS];


MEM_ADDR get_new_stack_frame(int number, void (*entry_point) (PROCESS, PARAM)) {
    MEM_ADDR stack = (MEM_ADDR) 0xA0000 - (30720 * number);
    poke_l(stack + 28, (LONG) entry_point);

    return stack;
}

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM), int prio, PARAM param, char *name) {
    int i;
    for(i = 1; i < MAX_PROCS; i++) {
        if(pcb[i].used == FALSE) {
            pcb[i].magic = MAGIC_PCB;
            pcb[i].used = TRUE;
            pcb[i].priority = prio;
            pcb[i].state = STATE_READY;
            pcb[i].esp = get_new_stack_frame(i, ptr_to_new_proc);
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

void print_process(WINDOW* wnd, PROCESS p) {

}

void print_all_processes(WINDOW* wnd) {

}

void init_process() {
    pcb[0].magic        = MAGIC_PCB;
    pcb[0].used         = TRUE;
    pcb[0].state        = STATE_READY;
    pcb[0].priority     = 1;
    pcb[0].first_port   = NULL;
    pcb->name           = "Boot process";

    add_ready_queue(&pcb[0]);

    int i;
    for(i = 1; i < MAX_PROCS; i++) {
        pcb[i].used = FALSE;
    }
}
