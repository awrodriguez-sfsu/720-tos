#include <kernel.h>
#include "../include/kernel.h"

/*
 * order to set switches
 * 5 -> green
 * 8 -> green
 * 9 -> red
 * 4 -> green
 * 1 -> green
 * 7 -> red
 *
 * tracks that zamboni can be on
 * [6, 7, 10, 13, 14, 15, 3, 4]
 */

#define SLEEP_TIME 15
WINDOW train_window;

void send_train_message(char* output, char* result, int result_length) {
    COM_Message msg;
    msg.output_buffer = output;
    msg.len_input_buffer = result_length;
    msg.input_buffer = result;

    send(com_port, &msg);

    sleep(SLEEP_TIME);
}

/*
 * Speed can be 0 - 5 in char format
 * 0 == stop
 * 5 == fast
 */
void set_speed(short speed) {
    char set_speed_msg[7];
    memset_b((MEM_ADDR) set_speed_msg, 0, 6);

    set_speed_msg[0] = 'L';
    set_speed_msg[1] = '2';
    set_speed_msg[2] = '0';
    set_speed_msg[3] = 'S';
    set_speed_msg[4] = (char) (speed + 48);
    set_speed_msg[5] = '\015';

    send_train_message(set_speed_msg, "", 0);
}

/*
 * Changes train direction
 */
void change_direction() {
    char change_direction_msg[6];
    memset_b((MEM_ADDR) change_direction_msg, 0, 5);

    change_direction_msg[0] = 'L';
    change_direction_msg[1] = '2';
    change_direction_msg[2] = '0';
    change_direction_msg[3] = 'D';
    change_direction_msg[4] = '\015';

    send_train_message(change_direction_msg, "", 0);
}

/*
 * switch_number can be 1 - 9 in char format
 * valid states are R or G in char format
 */
void change_switch_state(short switch_number, char state) {
    char change_switch_state_msg[5];
    memset_b((MEM_ADDR) change_switch_state_msg, 0, 4);

    change_switch_state_msg[0] = 'M';
    change_switch_state_msg[1] = (char) (switch_number + 48);
    change_switch_state_msg[2] = state;
    change_switch_state_msg[3] = '\015';

    send_train_message(change_switch_state_msg, "", 0);
}

void clear_buffer() {
    char clear_buffer_msg[3];
    memset_b((MEM_ADDR) clear_buffer_msg, 0, 2);

    clear_buffer_msg[0] = 'R';
    clear_buffer_msg[1] = '\015';

    send_train_message(clear_buffer_msg, "", 0);
}

/*
 * track_numbers can be 1 - 16
 */
BOOL check_contact(int track_number) {
    clear_buffer();

    if(track_number < 10) {
        char contact_response_1[3];
        memset_b((MEM_ADDR) contact_response_1, 0, 3);

        char contact_msg_lt_10[4];
        memset_b((MEM_ADDR) contact_msg_lt_10, 0, 3);

        contact_msg_lt_10[0] = 'C';
        contact_msg_lt_10[1] = (char) (track_number + 48);
        contact_msg_lt_10[2] = '\015';

        send_train_message(contact_msg_lt_10, contact_response_1, 3);
        return contact_response_1[1] == '1';
    } else {
        char contact_response_2[3];
        memset_b((MEM_ADDR) contact_response_2, 0, 3);

        char number[2];
        number[0] = '1';
        number[1] = (char) ((track_number % 10) + 48);

        char contact_msg_gt_9[5];
        memset_b((MEM_ADDR) contact_msg_gt_9, 0, 4);

        contact_msg_gt_9[0] = 'C';
        contact_msg_gt_9[1] = number[0];
        contact_msg_gt_9[2] = number[1];
        contact_msg_gt_9[3] = '\015';

        send_train_message(contact_msg_gt_9, contact_response_2, 3);
        return contact_response_2[1] == '1';
    }
}

/*
 * order to set switches
 * 5 -> green
 * 8 -> green
 * 9 -> red
 * 4 -> green
 * 1 -> green
 * 7 -> red ?????
 */
void track_safety() {
    change_switch_state(5, 'G');
    change_switch_state(8, 'G');
    change_switch_state(9, 'R');
    change_switch_state(4, 'G');
    change_switch_state(1, 'G');
}

void run_test_configuration() {

}

BOOL wait_for_contact(int track_number, BOOL time_limit, int limit) {
    if(time_limit) {
        while(limit > 0) {
            if(check_contact(track_number)) {
                return TRUE;
            } else {
                limit--;
            }
        }

        return FALSE;
    } else {
        while(!check_contact(track_number));
        return TRUE;
    }
}

//**************************
//run the train application
//**************************

void run_configuration1(BOOL zamboni) {

}

void run_configuration2(BOOL zamboni) {

}

void run_configuration3(BOOL zamboni) {

}

void run_configuration4(BOOL zamboni) {

}

/*
 * tracks that zamboni can be on
 * [6, 7, 10, 13, 14, 15, 3, 4]
 */
BOOL check_for_zamboni() {
    return wait_for_contact(3, TRUE, 25);
}

void train_process(PROCESS self, PARAM param) {
    int i;

    track_safety();
    BOOL zamboni = check_for_zamboni();

    kprintf("found zamboni: %d", zamboni);

    remove_ready_queue(self);
    for( i = 1; i > MAX_PROCS; i++) {
        if(&pcb[i] == self) {
            pcb[i].used = FALSE;
            break;
        }
    }
    resign();
}

void init_train(WINDOW* wnd) {
    train_window = *wnd;
    create_process(train_process, 4, 0, "Train Proceess");
}
