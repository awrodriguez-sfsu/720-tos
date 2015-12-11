#include <kernel.h>
#include "../include/kernel.h"

#define SLEEP_TIME 15

#define set_speed(speed) "L20S"speed"\015"
#define change_direction "L20D\015"
#define change_switch_green(switch) "M"switch"G\015"
#define change_switch_red(switch) "M"switch"R\015"
#define clear_buffer "R\015"
#define check_contact(contact) "C"contact"\015"

WINDOW train_window;

/*char message_buffer[3];*/

void create_com_message(COM_Message* com_message, char* message_buffer, BOOL reply, char* output) {
    volatile int lock;
    DISABLE_INTR(lock);

    com_message->output_buffer = output;

    if(reply) {
        com_message->len_input_buffer = 3;
        com_message->input_buffer = message_buffer;
    } else {
        com_message->input_buffer = NULL;
        com_message->len_input_buffer = 0;
    }

    ENABLE_INTR(lock);
}

void send_train_message(COM_Message* com_message) {
    send(com_port, com_message);
}

//**************************
//run the train application
//**************************

void run_configuration1(BOOL zamboni) {
    if(TRUE) {
        COM_Message com_message;
        create_com_message(&com_message, "", FALSE, change_switch_red("5"));
        send_train_message(&com_message);

        COM_Message com_message2;
        create_com_message(&com_message2, "", FALSE, change_switch_red("6"));
        send_train_message(&com_message2);
    } else {
        COM_Message com_message;
        create_com_message(&com_message, "", FALSE, set_speed("5"));
        send_train_message(&com_message);
    }
}

void run_configuration2(BOOL zamboni) {

}

void run_configuration3(BOOL zamboni) {

}

void run_configuration4(BOOL zamboni) {

}

BOOL check_for_zamboni() {
    COM_Message clear_message;
    COM_Message check_contact_4_message;

    char reply[3];

    create_com_message(&clear_message, "", FALSE, clear_buffer);
    send(com_port, &clear_message);

    sleep(SLEEP_TIME);

    create_com_message(&check_contact_4_message, reply, TRUE, check_contact("4"));
    send(com_port, &check_contact_4_message);

    sleep(SLEEP_TIME);

    return reply[1] == '1';
}

BOOL check_zamboni_direction() {
    sleep(SLEEP_TIME);
    return TRUE;
}

void train_process(PROCESS self, PARAM param) {
    char buffer[3];

    COM_Message msg;
    msg.output_buffer = "R\015";
    msg.len_input_buffer = 0;
    msg.input_buffer = buffer;
    send_train_message(&msg);

    sleep(SLEEP_TIME);

    msg.output_buffer = "C4\015";
    msg.input_buffer = buffer;
    msg.len_input_buffer = 3;
    send_train_message(&msg);

    if(buffer[1] == '1') {
        kprintf("found zamboni at the begining\n");
    } else {
        kprintf("did not find zamboni at the begining\n");
    }

    /*BOOL zamboni = check_for_zamboni();

    if(zamboni) {
        BOOL direction = check_zamboni_direction();
    }

    run_configuration1(zamboni);*/

    while(TRUE);
}

void init_train(WINDOW* wnd) {
    train_window = *wnd;
    create_process(train_process, 4, 0, "Train Proceess");
}
