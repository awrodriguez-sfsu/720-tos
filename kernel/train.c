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

#define SLEEP_TIME 20
#define ZAMBONI_LOOP_TIME 20

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
    memset_b((MEM_ADDR) set_speed_msg, 0, 7);

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
    memset_b((MEM_ADDR) change_direction_msg, 0, 6);

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
    memset_b((MEM_ADDR) change_switch_state_msg, 0, 5);

    change_switch_state_msg[0] = 'M';
    change_switch_state_msg[1] = (char) (switch_number + 48);
    change_switch_state_msg[2] = state;
    change_switch_state_msg[3] = '\015';

    send_train_message(change_switch_state_msg, "", 0);
}

void clear_buffer() {
    char clear_buffer_msg[3];
    memset_b((MEM_ADDR) clear_buffer_msg, 0, 3);

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
        char contact_response_1[4];
        memset_b((MEM_ADDR) contact_response_1, 0, 4);

        char contact_msg_lt_10[4];
        memset_b((MEM_ADDR) contact_msg_lt_10, 0, 4);

        contact_msg_lt_10[0] = 'C';
        contact_msg_lt_10[1] = (char) (track_number + 48);
        contact_msg_lt_10[2] = '\015';

        send_train_message(contact_msg_lt_10, contact_response_1, 3);
        return contact_response_1[1] == '1';
    } else {
        char contact_response_2[4];
        memset_b((MEM_ADDR) contact_response_2, 0, 4);

        char number[3];
        number[0] = '1';
        number[1] = (char) ((track_number % 10) + 48);

        char contact_msg_gt_9[5];
        memset_b((MEM_ADDR) contact_msg_gt_9, 0, 5);

        contact_msg_gt_9[0] = 'C';
        contact_msg_gt_9[1] = number[0];
        contact_msg_gt_9[2] = number[1];
        contact_msg_gt_9[3] = '\015';

        send_train_message(contact_msg_gt_9, contact_response_2, 3);
        return contact_response_2[1] == '1';
    }
}

BOOL wait_for_contact_no_limit(int track_number) {
    while(!check_contact(track_number));
    return TRUE;
}

BOOL wait_for_contact_with_limit(int track_number, int limit) {
    while(limit > 0) {
        if(check_contact(track_number)) {
            return TRUE;
        }

        limit--;
    }

    return FALSE;
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

/*
 * tracks that zamboni can be on
 * [6, 7, 10, 13, 14, 15, 3, 4]
 */
BOOL check_for_zamboni() {
    return wait_for_contact_with_limit(10, ZAMBONI_LOOP_TIME);
}

BOOL check_zamboni_direction(BOOL zamboni) {
    if(!zamboni) {
        return TRUE;
    }

    while(TRUE) {
        return wait_for_contact_with_limit(14, 5);
    }
}

typedef struct _track_config {
    int config;
    BOOL zamboni;
} Track_Configuration;

Track_Configuration determine_configuration() {
    BOOL zamboni = check_for_zamboni();
    BOOL moving_right = check_zamboni_direction(zamboni);

    Track_Configuration configuration;
    configuration.zamboni = zamboni;

    if(check_contact(2)) {
        if(moving_right) {
            configuration.config = 1;
            return configuration;
        } else {
            configuration.config = 2;
            return configuration;
        }
    } else if(check_contact(11)) {
        configuration.config = 3;
        return configuration;
    } else if(check_contact(16)) {
        configuration.config = 4;
        return configuration;
    } else {
        configuration.config = 0;
        return configuration;
    }
}

void attempt_retrieval(BOOL zamboni, int wait_time) {
    if(zamboni) {

    } else {
        set_speed(4);

        sleep(wait_time);

        set_speed(0);
        change_direction();
        set_speed(4);

        wait_for_contact_no_limit(14);
    }
}

//**************************
//run the train application
//**************************

void run_configuration1(BOOL zamboni) {
    if(zamboni) {
        wait_for_contact_no_limit(7);
        change_switch_state(6, 'G');
        change_switch_state(5, 'R');
        wait_for_contact_no_limit(9);
    }

    change_switch_state(4, 'R');
    change_switch_state(3, 'G');
    set_speed(5);

    wait_for_contact_no_limit(6);
    set_speed(4);

    wait_for_contact_no_limit(1);
    set_speed(0);

    change_switch_state(5, 'R');
    change_switch_state(6, 'R');

    change_direction();
    set_speed(5);

    wait_for_contact_no_limit(8);
    set_speed(0);
}

void run_configuration2(BOOL zamboni) {
    if(zamboni) {
        wait_for_contact_no_limit(3);
        change_switch_state(8, 'R');
        wait_for_contact_no_limit(14);
    }

    change_switch_state(4, 'R');
    change_switch_state(3, 'G');
    set_speed(5);

    wait_for_contact_no_limit(6);
    set_speed(4);

    wait_for_contact_no_limit(1);
    set_speed(0);

    change_switch_state(5, 'R');
    change_switch_state(6, 'R');

    change_direction();
    set_speed(5);

    wait_for_contact_no_limit(8);
    set_speed(0);
}

void run_configuration3(BOOL zamboni) {
    if(zamboni) {
        wait_for_contact_no_limit(10);
        set_speed(5);

        change_switch_state(5, 'R');
        change_switch_state(6, 'G');
        wait_for_contact_no_limit(7);
        set_speed(4);

        wait_for_contact_no_limit(9);
        change_switch_state(5, 'G');

        wait_for_contact_no_limit(12);
        set_speed(0);

        change_switch_state(7, 'R');
        wait_for_contact_no_limit(14);
        change_direction();
        set_speed(5);

        wait_for_contact_no_limit(14);
        set_speed(4);
        change_switch_state(1, 'R');
        change_switch_state(2, 'R');

        wait_for_contact_no_limit(12);
        set_speed(0);
        change_switch_state(1, 'G');
        change_switch_state(7, 'G');

        wait_for_contact_no_limit(10);
        set_speed(5);

        change_switch_state(1, 'R');
        change_switch_state(2, 'R');
        change_switch_state(7, 'R');

        wait_for_contact_no_limit(7);
        set_speed(4);
        change_switch_state(4, 'R');
        change_switch_state(3, 'R');

        wait_for_contact_no_limit(5);
        set_speed(0);
    } else {
        set_speed(5);
        change_switch_state(5, 'R');
        change_switch_state(6, 'G');
        change_switch_state(8, 'R');

        wait_for_contact_no_limit(12);
        wait_for_contact_no_limit(14);
        wait_for_contact_no_limit(12);
        change_switch_state(8, 'G');

        change_switch_state(4, 'R');
        change_switch_state(3, 'R');

        wait_for_contact_no_limit(6);
        set_speed(4);

        wait_for_contact_no_limit(5);
        set_speed(0);
    }
}

void run_configuration4(BOOL zamboni) {
    if(zamboni) {
        wait_for_contact_no_limit(3);
        set_speed(4);
        wait_for_contact_no_limit(6);
        set_speed(0);
        change_direction();
        set_speed(5);

        wait_for_contact_no_limit(14);
        set_speed(0);
        change_direction();

        change_switch_state(9, 'G');
        set_speed(5);
        sleep(750);
        change_switch_state(9, 'R');
        set_speed(0);
        change_direction();

        wait_for_contact_no_limit(14);
        change_switch_state(8, 'R');
        set_speed(5);

        wait_for_contact_no_limit(13);
        change_switch_state(8, 'G');

        wait_for_contact_no_limit(10);
        change_switch_state(8, 'R');

        wait_for_contact_no_limit(7);
        set_speed(4);

        change_switch_state(4, 'R');
        change_switch_state(3, 'R');

        wait_for_contact_no_limit(5);
        set_speed(0);
    } else {
        int wait_time = 500;

        change_switch_state(5, 'G');
        change_switch_state(9, 'G');
        set_speed(5);

        wait_for_contact_no_limit(14);
        set_speed(0);

        while(check_contact(16)) {
            attempt_retrieval(zamboni, wait_time);
            wait_time += 50;
        }

        change_switch_state(8, 'G');
        set_speed(5);
        change_switch_state(4, 'R');
        change_switch_state(3, 'R');

        wait_for_contact_no_limit(6);
        set_speed(4);

        wait_for_contact_no_limit(5);
        set_speed(0);
    }
}

void run_train() {
    track_safety();

    Track_Configuration configuration = determine_configuration();

    switch(configuration.config) {
        case 1:
            run_configuration1(configuration.zamboni);
            break;
        case 2:
            run_configuration2(configuration.zamboni);
            break;
        case 3:
            run_configuration3(configuration.zamboni);
            break;
        case 4:
            run_configuration4(configuration.zamboni);
            break;
        default:
            kprintf("could not determine configuration\n");
    }
}

void train_process(PROCESS self, PARAM param) {

    run_train();

    remove_ready_queue(self);

    int i;
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
