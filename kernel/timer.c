#include <kernel.h>
#include "../include/kernel.h"

PORT timer_port;

int ticks_remaining[MAX_PROCS];

void timer_notifier(PROCESS self, PARAM param) {
    PORT timer_notifier_port = (PORT) param;

    Timer_Message new_message;
    new_message.num_of_ticks = NULL;

    while(TRUE) {
        wait_for_interrupt(TIMER_IRQ);
        message(timer_notifier_port, &new_message);
    }
}

void timer_process(PROCESS self, PARAM param) {
    PORT timer_notifier_port = create_port();
    create_process(timer_notifier, 7, (PARAM) timer_notifier_port, "Timer Service Notifier");

    int i;
    PROCESS client_process;

    while(TRUE) {
        Timer_Message* message = (Timer_Message*) receive(&client_process);

        if(message->num_of_ticks == NULL) {
            for(i = 0; i < MAX_PROCS; i++) {
                if(ticks_remaining[i] != NULL) {
                    ticks_remaining[i]--;
                    if(ticks_remaining[i] == 0) {
                        ticks_remaining[i] = NULL;
                        reply(&pcb[i]);
                    }
                }
            }
        } else {
            i = client_process - pcb;
            assert(client_process == &pcb[i]);
            ticks_remaining[i] = message->num_of_ticks;
            continue;
        }
    }

}


void sleep(int ticks) {
    Timer_Message new_message;
    new_message.num_of_ticks = ticks;
    send(timer_port, &new_message);
}

void init_timer () {
    timer_port = create_process(timer_process, 6, 0, "Timer Service");

    int i;
    for(i = 0; i < MAX_PROCS; i++) {
        ticks_remaining[i] = NULL;
    }
}