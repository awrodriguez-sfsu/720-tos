#include <kernel.h>
#include "../include/kernel.h"

#define ENTER_KEY 13
#define BACKSPACE 8

WINDOW train_window = {0, 0, 61, 9, 0, 0, ' '};
WINDOW shell_window = {0, 9, 61, 16, 0, 0, 0xDB};
WINDOW pacman_window = {61, 9, 18, 16, 0, 0, ' '};

BOOL train_running = FALSE;

char command[256];
unsigned short command_length;

void reset_command() {
    /* reset command and command length*/
    memset_b((MEM_ADDR) command, 0, command_length);
    command_length = 0;
}

void shell_print_ascii() {
    kprintf("             ____________________________________________________\n");
    kprintf("            /                                                    \\\n");
    kprintf("           |    _____________________________________________     |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |  Welcome to TOS!                            |    |\n");
    kprintf("           |   |  By: Anthony Rodriguez                      |    |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |                                             |    |\n");
    kprintf("           |   |_____________________________________________|    |\n");
    kprintf("           |                                                      |\n");
    kprintf("            \\_____________________________________________________/\n");
    kprintf("                   \\_______________________________________/\n");
    kprintf("             _____________________________________________________\n");
    kprintf("          _-'.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.--.  .-.-.`-_\n");
    kprintf("       _-'.-.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-`__`. .-.-.-.`-_\n");
    kprintf("    _-'.-.-.-.-. .-----.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-----. .-.-.-.-.`-_\n");
    kprintf(" _-'.-.-.-.-.-. .---.-. .-----------------------------. .-.---. .---.-.-.-.`-_\n");
    kprintf(":-----------------------------------------------------------------------------:\n");
    kprintf("`---._.-----------------------------------------------------------------._.---'\n");

    char ch;
    Keyb_Message msg;
    msg.key_buffer = &ch;
    send(keyb_port, &msg);

    clear_window(kernel_window);
}

void shell_print_welcome() {
    wprintf(&shell_window, "============= Welcome to TOS =============\n");
    wprintf(&shell_window, "=====Type help for a list of commands=====\n");
}

void shell_prompt() {
    wprintf(&shell_window, ">> ");
}

void shell_help() {
    wprintf(&shell_window, "List of Commands\n");
    wprintf(&shell_window, "----------------------------------------\n");
    wprintf(&shell_window, "help           Print list of commands\n");
    wprintf(&shell_window, "clear          Clear window\n");
    wprintf(&shell_window, "ps             Print all processes\n");
    wprintf(&shell_window, "train run      Run the train application\n");
    wprintf(&shell_window, "train go       Run the train application\n");
    wprintf(&shell_window, "train stop     Run the train application\n");
    wprintf(&shell_window, "ascii-art      Display Startup ascii art\n");
    wprintf(&shell_window, "pacman         Pacman\n");
    wprintf(&shell_window, "\n");
}

void shell_clear() {
    clear_window(&shell_window);
}

void shell_print() {
    print_all_processes(&shell_window);
}

void shell_train() {
    if(train_running) {
        wprintf(&shell_window, "Train Application is already running\n");
    } else {
        init_train(&train_window);
        train_running = TRUE;
    }
}

void shell_not_recognized() {
    wprintf(&shell_window, "(%s) was not recognized\n", command);
}

void execute(char* command, unsigned short command_length) {
    wprintf(&shell_window, "\n");

    if(k_strcmp(command, "help")) {
        shell_help();
    } else if(k_strcmp(command, "clear")) {
        shell_clear();
    } else if(k_strcmp(command, "ps")) {
        shell_print();
    } else if(k_strcmp(command, "train run")) {
        clear_window(kernel_window);
        shell_train();
    } else if(k_strcmp(command, "train go")) {
        set_speed(5);
    } else if(k_strcmp(command, "train stop")) {
        set_speed(0);
    } else if(k_strcmp(command, "ascii-art")) {
        shell_print_ascii();
    } else if(k_strcmp(command, "pacman")) {
        init_pacman(&pacman_window, 5);
    } else {
        shell_not_recognized();
    }

    reset_command();
    shell_prompt();
    show_cursor(&shell_window);
}

void shell_process(PROCESS self, PARAM param) {

    reset_command();
    clear_window(kernel_window);
    shell_print_ascii();
    shell_print_welcome();
    shell_prompt();

    while(TRUE) {
        char ch;
        Keyb_Message keyb_message;

        keyb_message.key_buffer = &ch;
        send(keyb_port, &keyb_message);

        if(ch == ENTER_KEY) {
            execute(command, command_length);
        } else if(ch == BACKSPACE) {
            backspace(&shell_window, 3);
            if(command_length > 0) {
                command[command_length - 1] = 0;
                command_length--;
            } else {
                reset_command();
            }
        } else if(command_length == 256) {
            WINDOW error_window = {0, 24, 80, 1, 0, 0, ' '};
            wprintf(&error_window, "This shell does accept commands longer than 256 characters");
        } else {
            command[command_length] = ch;
            command_length++;
            wprintf(&shell_window, "%c", ch);
        }
    }
}

void init_shell() {
    create_process(shell_process, 6, 0, "TOS Shell");
}
