#include <kernel.h>
#include "../include/kernel.h"

PORT_DEF port[MAX_PORTS];

PORT create_port() {
    return create_new_port(active_proc);
}

PORT create_new_port (PROCESS owner) {
    int i;
    for(i = 0; i < MAX_PORTS; i++) {
        if(port[i].used == FALSE) {
            port[i].magic = MAGIC_PORT;
            port[i].used = TRUE;
            port[i].open = TRUE;
            port[i].owner = owner;
            port[i].blocked_list_head = NULL;
            port[i].blocked_list_tail = NULL;
            port[i].next = NULL;

            return &port[i];
        }
    }
}

void open_port (PORT port) {
    port->open = TRUE;
}

void close_port (PORT port) {
    port->open = FALSE;
}

void send (PORT dest_port, void* data) {

}

void message (PORT dest_port, void* data) {

}

void* receive (PROCESS* sender) {

}

void reply (PROCESS sender) {

}

void init_ipc() {
    int i;
    for(i = 0; i < MAX_PORTS; i++) {
        port[i].used = FALSE;
    }
}
