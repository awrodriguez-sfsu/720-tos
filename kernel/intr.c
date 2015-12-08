#include <kernel.h>
#include "../include/kernel.h"

BOOL interrupts_initialized = FALSE;

IDT idt[MAX_INTERRUPTS];
PROCESS interrupt_table[MAX_INTERRUPTS];

void load_idt (IDT* base) {
    unsigned short           limit;
    volatile unsigned char   mem48 [6];
    volatile unsigned       *base_ptr;
    volatile short unsigned *limit_ptr;

    limit      = MAX_INTERRUPTS * IDT_ENTRY_SIZE - 1;
    base_ptr   = (unsigned *) &mem48[2];
    limit_ptr  = (short unsigned *) &mem48[0];
    *base_ptr  = (unsigned) base;
    *limit_ptr = limit;
    asm ("lidt %0" : "=m" (mem48));
}

void init_idt_entry (int intr_no, void (*isr) (void)) {
    idt[intr_no].offset_0_15  = (unsigned short) ((unsigned short) isr & 0xFFFF);
    idt[intr_no].offset_16_31 = (unsigned short) (((unsigned short) isr >> 16) & 0xFFFF);
    idt[intr_no].selector     = CODE_SELECTOR;
    idt[intr_no].dword_count  = 0x00;
    idt[intr_no].unused       = 0x00;
    idt[intr_no].type         = 0x0E;
    idt[intr_no].dt           = 0x00;
    idt[intr_no].dpl          = 0x00;
    idt[intr_no].p            = 0x01;
}

void dummy_isr() {
    asm("movb $0x20,%al");
    asm("outb %al,$0x20");
    asm("iret");
}

void fatal_exception(int interrupt_number) {
    WINDOW error_window = {0, 24, 80, 1, 0, 0, ' '};

    wprintf(&error_window, "Fatal Exception %d (%s)", interrupt_number, active_proc->name);
    while(1);
}

void exception0() {
    fatal_exception(0);
}

void exception1() {
    fatal_exception(1);
}

void exception2() {
    fatal_exception(2);
}

void exception3() {
    fatal_exception(3);
}

void exception4() {
    fatal_exception(4);
}

void exception5() {
    fatal_exception(5);
}

void exception6() {
    fatal_exception(6);
}

void exception7() {
    fatal_exception(7);
}

void exception8() {
    fatal_exception(8);
}

void exception9() {
    fatal_exception(9);
}

void exception10() {
    fatal_exception(10);
}

void exception11() {
    fatal_exception(11);
}

void exception12() {
    fatal_exception(12);
}

void exception13() {
    fatal_exception(13);
}

void exception14() {
    fatal_exception(14);
}

void exception15() {
    fatal_exception(15);
}

void exception16() {
    fatal_exception(16);
}

void isr_timer_handler() {
    volatile int lock;
    DISABLE_INTR(lock);

    PROCESS process = interrupt_table[TIMER_IRQ];

    if(process != NULL) {
        change_state(process, STATE_READY);
        interrupt_table[TIMER_IRQ] = NULL;
    }

    active_proc = dispatcher();

    ENABLE_INTR(lock);
}

void isr_timer_wrapper() {
    /* save context */
    asm("pushl %eax");
    asm("pushl %ecx");
    asm("pushl %edx");
    asm("pushl %ebx");
    asm("pushl %ebp");
    asm("pushl %esi");
    asm("pushl %edi");

    asm("movl %%esp, %0" : "=m" (active_proc->esp) : );
    isr_timer_handler();
    asm("movl %0, %%esp" : : "m" (active_proc->esp));

    /* Reset interrupt controller */
    asm("movb $0x20,%al");
    asm("outb %al,$0x20");

    /* load context */
    asm("popl %edi");
    asm("popl %esi");
    asm("popl %ebp");
    asm("popl %ebx");
    asm("popl %edx");
    asm("popl %ecx");
    asm("popl %eax");

    asm("iret");
}

/*
 * COM1 ISR
 */
void isr_com1_handler() {

}

void isr_com1_wrapper() {
    /* save context */
    asm("pushl %eax");
    asm("pushl %ecx");
    asm("pushl %edx");
    asm("pushl %ebx");
    asm("pushl %ebp");
    asm("pushl %esi");
    asm("pushl %edi");

    asm("movl %%esp, %0" : "=m" (active_proc->esp) : );
    isr_com1_handler();
    asm("movl %0, %%esp" : : "m" (active_proc->esp));

    /* Reset interrupt controller */
    asm("movb $0x20,%al");
    asm("outb %al,$0x20");

    /* load context */
    asm("popl %edi");
    asm("popl %esi");
    asm("popl %ebp");
    asm("popl %ebx");
    asm("popl %edx");
    asm("popl %ecx");
    asm("popl %eax");

    asm("iret");
}

/*
 * Keyboard ISR
 */
void isr_keyb_handler() {

}

void isr_keyb_wrapper() {
    /* save context */
    asm("pushl %eax");
    asm("pushl %ecx");
    asm("pushl %edx");
    asm("pushl %ebx");
    asm("pushl %ebp");
    asm("pushl %esi");
    asm("pushl %edi");

    asm("movl %%esp, %0" : "=m" (active_proc->esp) : );
    isr_keyb_handler();
    asm("movl %0, %%esp" : : "m" (active_proc->esp));

    /* Reset interrupt controller */
    asm("movb $0x20,%al");
    asm("outb %al,$0x20");

    /* load context */
    asm("popl %edi");
    asm("popl %esi");
    asm("popl %ebp");
    asm("popl %ebx");
    asm("popl %edx");
    asm("popl %ecx");
    asm("popl %eax");

    asm("iret");
}

BOOL invalid_interrupt(int intr_no) {
    return !(intr_no == TIMER_IRQ || intr_no == COM1_IRQ || intr_no == KEYB_IRQ);
}

void wait_for_interrupt (int intr_no) {
    volatile int lock;
    DISABLE_INTR(lock);

    if(active_proc->state == STATE_INTR_BLOCKED || invalid_interrupt(intr_no)) {
        ENABLE_INTR(lock);
        return;
    }

    while(interrupt_table[intr_no]->state == STATE_INTR_BLOCKED) {
        ENABLE_INTR(lock);
        resign();
    }

    interrupt_table[intr_no] = active_proc;
    change_state(active_proc, STATE_INTR_BLOCKED);

    ENABLE_INTR(lock);
    resign();
}

void delay () {
    asm ("nop;nop;nop");
}

void re_program_interrupt_controller () {
    /* Shift IRQ Vectors so they don't collide with the
       x86 generated IRQs */

    // Send initialization sequence to 8259A-1
    asm ("movb $0x11,%al;outb %al,$0x20;call delay");
    // Send initialization sequence to 8259A-2
    asm ("movb $0x11,%al;outb %al,$0xA0;call delay");
    // IRQ base for 8259A-1 is 0x60
    asm ("movb $0x60,%al;outb %al,$0x21;call delay");
    // IRQ base for 8259A-2 is 0x68
    asm ("movb $0x68,%al;outb %al,$0xA1;call delay");
    // 8259A-1 is the master
    asm ("movb $0x04,%al;outb %al,$0x21;call delay");
    // 8259A-2 is the slave
    asm ("movb $0x02,%al;outb %al,$0xA1;call delay");
    // 8086 mode for 8259A-1
    asm ("movb $0x01,%al;outb %al,$0x21;call delay");
    // 8086 mode for 8259A-2
    asm ("movb $0x01,%al;outb %al,$0xA1;call delay");
    // Don't mask IRQ for 8259A-1
    asm ("movb $0x00,%al;outb %al,$0x21;call delay");
    // Don't mask IRQ for 8259A-2
    asm ("movb $0x00,%al;outb %al,$0xA1;call delay");
}

void init_interrupts() {
    int i;

    load_idt(idt);

    for(i = 17; i < MAX_INTERRUPTS; i++) {
        init_idt_entry(i, dummy_isr);
        interrupt_table[i] = NULL;
    }

    init_idt_entry(0,  exception0);
    init_idt_entry(1,  exception1);
    init_idt_entry(2,  exception2);
    init_idt_entry(3,  exception3);
    init_idt_entry(4,  exception4);
    init_idt_entry(5,  exception5);
    init_idt_entry(6,  exception6);
    init_idt_entry(7,  exception7);
    init_idt_entry(8,  exception8);
    init_idt_entry(9,  exception9);
    init_idt_entry(10, exception10);
    init_idt_entry(11, exception11);
    init_idt_entry(12, exception12);
    init_idt_entry(13, exception13);
    init_idt_entry(14, exception14);
    init_idt_entry(15, exception15);
    init_idt_entry(16, exception16);

    init_idt_entry(TIMER_IRQ, isr_timer_wrapper);

    re_program_interrupt_controller();

    interrupts_initialized = TRUE;

    asm("sti");
}
