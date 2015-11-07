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

void remove_from_port_queue(PORT port, PROCESS proc) {
    PROCESS current = port->blocked_list_head;

    if(current == proc) {
        if(current->next_blocked != NULL) {
            port->blocked_list_head = current->next_blocked;
        } else {
            port->blocked_list_head = NULL;
            port->blocked_list_tail = NULL;
        }
    } else {
        while(current->next_blocked != proc) {
            current = current->next_blocked;
        }

        current->next_blocked = current->next_blocked->next_blocked;

        if(port->blocked_list_tail == proc) {
            port->blocked_list_tail = current;
        }
    }
}

void add_to_port_queue(PORT port, PROCESS proc) {
    if(port->blocked_list_head == NULL) {
        port->blocked_list_head = proc;
    } else {
        port->blocked_list_tail->next_blocked = proc;
    }

    port->blocked_list_tail = proc;
    proc->next_blocked = NULL;
}

void send (PORT dest_port, void* data) {
    PROCESS receiver = dest_port->owner;
    active_proc->param_proc = receiver;
    active_proc->param_data = data;

    if(receiver->state == STATE_RECEIVE_BLOCKED && dest_port->open) {
        change_state(receiver, STATE_READY);
        change_state(active_proc, STATE_REPLY_BLOCKED);
    } else {
        change_state(active_proc, STATE_SEND_BLOCKED);
    }

    add_to_port_queue(dest_port, active_proc);

    resign();
}

void message (PORT dest_port, void* data) {
    PROCESS receiver = dest_port->owner;
    active_proc->param_proc = receiver;
    active_proc->param_data = data;

    if(receiver->state == STATE_RECEIVE_BLOCKED && dest_port->open) {
        change_state(receiver, STATE_READY);
    } else {
        change_state(active_proc, STATE_MESSAGE_BLOCKED);
    }
    add_to_port_queue(dest_port, active_proc);

    resign();
}

BOOL message_pending() {
    PORT current_port = active_proc->first_port;

    while(current_port != NULL) {

        if(current_port->open && current_port->blocked_list_head != NULL) {
            return TRUE;
        }

        current_port = current_port->next;
    }

    return FALSE;
}

PORT get_next_message_port() {
    PORT current_port = active_proc->first_port;

    while(current_port != NULL) {
        if(current_port->open && current_port->blocked_list_head != NULL) {
            return current_port;
        }

        current_port = current_port->next;
    }

    return FALSE;
}

void* receive (PROCESS* sender) {
    if(!message_pending()) {
        change_state(active_proc, STATE_RECEIVE_BLOCKED);
        resign();
    }

    PORT current_port = get_next_message_port();
    PROCESS message_sender = current_port->blocked_list_head;

    if(message_sender->state == STATE_MESSAGE_BLOCKED) {
        change_state(message_sender, STATE_READY);
    } else if(message_sender->state == STATE_SEND_BLOCKED) {
        change_state(message_sender, STATE_REPLY_BLOCKED);
    }

    remove_from_port_queue(current_port, message_sender);

    *sender = message_sender;
    return message_sender->param_data;
}

void reply (PROCESS sender) {
    change_state(sender, STATE_READY);
    resign();
}

void init_ipc() {
    int i;
    for(i = 0; i < MAX_PORTS; i++) {
        port[i].used = FALSE;
    }
}
