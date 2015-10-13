#include <kernel.h>
#include "../include/kernel.h"

void poke_b (MEM_ADDR addr, BYTE value) {
    BYTE* dest = (BYTE*) addr;

    *dest = value;
}

void poke_w (MEM_ADDR addr, WORD value) {
    WORD* dest = (WORD*) addr;

    *dest = value;
}

void poke_l (MEM_ADDR addr, LONG value) {
    LONG* dest = (LONG*) addr;

    *dest = value;
}

BYTE peek_b (MEM_ADDR addr) {
    return * (BYTE*) addr;
}

WORD peek_w (MEM_ADDR addr) {
    return * (WORD*) addr;
}

LONG peek_l (MEM_ADDR addr) {
    return * (LONG*) addr;
}

void* memset_b(MEM_ADDR addr, BYTE value, int length) {
    MEM_ADDR destination = addr;
    while(length > 0) {
        poke_b(destination, value);
        destination++;

        length--;
    }

    return (void* ) addr;
}
