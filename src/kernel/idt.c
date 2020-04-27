#ifndef IDT_C
#define IDT_C 



// defining the idt entry
struct idt_entry_struct {
    unsigned short offset_lower;
    unsigned short segment_selector;
    unsigned char zeros;
    unsigned char flags;
    unsigned short offset_upper;
};
typedef struct idt_entry_struct idt_entry_type;


// defining the descriptor
struct idt_descriptor_struct{
    unsigned short limit; 
    unsigned int address;
};
typedef struct idt_descriptor_struct idt_descriptor_type;


// now we define our actual variable
idt_entry_type IDT[256];
idt_descriptor_type idt_descriptor;



extern void idt_load(unsigned int);
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



void set_idt_gate(int n, unsigned int handler_address)
{
    IDT[n].offset_lower = low_16(handler_address);
    IDT[n].segment_selector = KERNEL_CODE_SEGMENT;
    IDT[n].zeros = 0;
    IDT[n].flags = 0x8E; 
    IDT[n].offset_upper = high_16(handler_address);
}

void initIDT()
{
    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
    idt_descriptor.address = (unsigned int)(&IDT);

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

    idt_load( (unsigned int)&idt_descriptor );
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


void isr_handler()
{
    kprint("isr_handler\n");
}



#endif // IDT_C 