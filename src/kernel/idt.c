#ifndef IDT_C
#define IDT_C 


// defining the idt entry
struct idt_entry_struct {
    unsigned short offset_lower;
    unsigned short segment_selector;
    unsigned char zeros;
    unsigned char flags;
    unsigned short offset_upper;
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_type;


// defining the descriptor
struct idt_descriptor_struct{
    unsigned short limit; 
    unsigned int address;
} __attribute__((packed));

typedef struct idt_descriptor_struct idt_descriptor_type;


// now we define our actual variable
idt_entry_type IDT[256];
idt_descriptor_type idt_descriptor;


extern void load_idt(unsigned int);
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();


extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();

extern void test_idt();

void set_idt_gate(int n, unsigned int handler_address)
{
    IDT[n].offset_lower = low_16(handler_address);
    IDT[n].segment_selector = KERNEL_CODE_SEGMENT;
    IDT[n].zeros = 0;
    IDT[n].flags = 0x8E; 
    IDT[n].offset_upper = high_16(handler_address);
}

void test_interrupt()
{
    test_idt();
}

void initIDT()
{
    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
    idt_descriptor.address = (unsigned int)(&IDT);


    // Remap the PIC
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);

    set_idt_gate(0, (unsigned int)&isr0);
    set_idt_gate(1, (unsigned int)&isr1);
    set_idt_gate(2, (unsigned int)&isr2);
    set_idt_gate(3, (unsigned int)&isr3);
    set_idt_gate(4, (unsigned int)&isr4);
    set_idt_gate(5, (unsigned int)&isr5);
    set_idt_gate(6, (unsigned int)&isr6);
    set_idt_gate(7, (unsigned int)&isr7);
    set_idt_gate(8, (unsigned int)&isr8);
    set_idt_gate(9, (unsigned int)&isr9);
    set_idt_gate(10, (unsigned int)&isr10);
    set_idt_gate(11, (unsigned int)&isr11);
    set_idt_gate(12, (unsigned int)&isr12);
    set_idt_gate(13, (unsigned int)&isr13);
    set_idt_gate(14, (unsigned int)&isr14);
    set_idt_gate(15, (unsigned int)&isr15);
    set_idt_gate(16, (unsigned int)&isr16);
    set_idt_gate(17, (unsigned int)&isr17);
    set_idt_gate(18, (unsigned int)&isr18);
    set_idt_gate(19, (unsigned int)&isr19);
    set_idt_gate(20, (unsigned int)&isr20);
    set_idt_gate(21, (unsigned int)&isr21);
    set_idt_gate(22, (unsigned int)&isr22);
    set_idt_gate(23, (unsigned int)&isr23);
    set_idt_gate(24, (unsigned int)&isr24);
    set_idt_gate(25, (unsigned int)&isr25);
    set_idt_gate(26, (unsigned int)&isr26);
    set_idt_gate(27, (unsigned int)&isr27);
    set_idt_gate(28, (unsigned int)&isr28);
    set_idt_gate(29, (unsigned int)&isr29);
    set_idt_gate(30, (unsigned int)&isr30);
    set_idt_gate(31, (unsigned int)&isr31);


    set_idt_gate(32, (unsigned int)&irq0);
    set_idt_gate(33, (unsigned int)&irq1);
    set_idt_gate(34, (unsigned int)&irq2);
    set_idt_gate(35, (unsigned int)&irq3);
    set_idt_gate(36, (unsigned int)&irq4);

    load_idt( (unsigned int)&idt_descriptor );

    init_keyboard_driver();

    int b = 0;
}



/*
unsigned char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    
    
    "Reserved",
    "Reserved"
};

*/

void interrupt_handler_0()
{
    kprint("isr_handler0\n");
}

void interrupt_handler_1()
{
    kprint("isr_handler1\n");
}

void interrupt_handler_2()
{
    kprint("isr_handler2\n");
}

void interrupt_handler_3()
{
    kprint("isr_handler3\n");
}

void interrupt_handler_4()
{
    kprint("isr_handler4\n");
}

void interrupt_handler_5()
{
    kprint("isr_handler5\n");
}

void interrupt_handler_6()
{
    kprint("isr_handler6\n");
}

void interrupt_handler_7()
{
    kprint("isr_handler7\n");
}

void interrupt_handler_8()
{
    kprint("isr_handler\n");
}

void interrupt_handler_9()
{
    kprint("isr_handler\n");
}

void interrupt_handler_10()
{
    kprint("isr_handler\n");
}

void interrupt_handler_11()
{
    kprint("isr_handler\n");
}

void interrupt_handler_12()
{
    kprint("isr_handler\n");
}

void interrupt_handler_13()
{
    kprint("isr_handler\n");
}

void interrupt_handler_14()
{
    kprint("isr_handler\n");
}

void interrupt_handler_15()
{
    kprint("isr_handler\n");
}

void interrupt_handler_16()
{
    kprint("isr_handler\n");
}

void interrupt_handler_17()
{
    kprint("isr_handler\n");
}

void interrupt_handler_18()
{
    kprint("isr_handler\n");
}

void interrupt_handler_19()
{
    kprint("isr_handler\n");
}

void interrupt_handler_20()
{
    kprint("isr_handler\n");
}

void interrupt_handler_21()
{
    kprint("isr_handler\n");
}

void interrupt_handler_22()
{
    kprint("isr_handler\n");
}

void interrupt_handler_23()
{
    kprint("isr_handler\n");
}

void interrupt_handler_24()
{
    kprint("isr_handler\n");
}

void interrupt_handler_25()
{
    kprint("isr_handler\n");
}

void interrupt_handler_26()
{
    kprint("isr_handler\n");
}

void interrupt_handler_27()
{
    kprint("isr_handler\n");
}

void interrupt_handler_28()
{
    kprint("isr_handler\n");
}

void interrupt_handler_29()
{
    kprint("isr_handler\n");
}

void interrupt_handler_30()
{
    kprint("isr_handler\n");
}

void interrupt_handler_31()
{
    kprint("isr_handler\n");
}


void irq0_handler()
{
    port_byte_out(0x20, 0x20);
}

void irq1_handler()
{
    keyboard_handler();

    port_byte_out(0x20, 0x20);
}

void irq2_handler()
{
    port_byte_out(0x20, 0x20);
}

void irq3_handler()
{
    port_byte_out(0x20, 0x20);
}

void irq4_handler()
{
    port_byte_out(0x20, 0x20);
}


#endif // IDT_C 