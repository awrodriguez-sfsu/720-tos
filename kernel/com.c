/*
 * Internet ressources:
 * 
 * http://workforce.cup.edu/little/serial.html
 *
 * http://www.lammertbies.nl/comm/info/RS-232.html
 *
 */
#include <kernel.h>
#include "../include/kernel.h"

PORT com_port;

void com_reader_process(PROCESS self, PARAM param) {
    int i;

    PROCESS com_proc;
    while(TRUE) {
        COM_Message* client_message = (COM_Message*) receive(&com_proc);

        for(i = 0; i < client_message->len_input_buffer; i++) {
            wait_for_interrupt(COM1_IRQ);
            client_message->input_buffer[i] = inportb(COM1_PORT);
        }

        message(com_port, (void*) client_message);
    }
}

void com_process(PROCESS self, PARAM param) {
    PORT com_reader_port = create_process(com_reader_process, 7, 0, "Com Reader Process");

    int i;
    PROCESS client_process;
    PROCESS reader_process;

    while(TRUE) {
        COM_Message* client_message = (COM_Message*) receive(&client_process);
        message(com_reader_port, (void*) client_message);

        int length = k_strlen(client_message->output_buffer);

        for(i = 0; i < length; i++) {
            while(!(inportb(COM1_PORT + 5) & (1 << 5)));
            outportb(COM1_PORT, (unsigned char) client_message->output_buffer[i]);
        }

        client_message = (COM_Message*) receive(&reader_process);

        reply(client_process);
    }

}

void init_uart() {
    /* LineControl disabled to set baud rate */
    outportb (COM1_PORT + 3, 0x80);
    /* lower byte of baud rate */
    outportb (COM1_PORT + 0, 0x30);
    /* upper byte of baud rate */
    outportb (COM1_PORT + 1, 0x00);
    /* 8 Bits, No Parity, 2 stop bits */
    outportb (COM1_PORT + 3, 0x07);
    /* Interrupt enable*/
    outportb (COM1_PORT + 1, 1);
    /* Modem control */
    outportb (COM1_PORT + 4, 0x0b);
    inportb (COM1_PORT);
}

void init_com () {
    com_port = create_process(com_process, 6, 0, "Com Process");

    init_uart();
}
