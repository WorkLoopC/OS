/*
Make space for the interrupt descriptor table
Tell the CPU where that space is (see GDT Tutorial: lidt works the very same way as lgdt)
Tell the PIC that you no longer want to use the BIOS defaults (see Programming the PIC chips)
Write a couple of ISR handlers (see Interrupt Service Routines) for both IRQs and exceptions
Put the addresses of the ISR handlers in the appropriate descriptors (in Interrupt Descriptor Table)
Enable all supported interrupts in the IRQ mask (of the PIC)

Disable Interrupts
If they're enabled, be absolutely sure to turn them off or you could run into undesired behavior and exceptions. This can be achieved through the CLI assembly instruction.
Interrupt handling mostly revolves around IDT + TSS, not segmentation
*/
#include "ISR.h"
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_DS   0x18
#define USER_CS   0x20

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)


inline void exception_handler() {
    __asm__ volatile ("cli; hlt");
}


//_________________________________________________ Global Descriptor Table (GDT/GDTR) _________________________________________________
static struct __attribute__((packed)) GlobalDescriptor64 {
    uint16_t limit;    //A 20-bit value, tells the maximum addressable unit, either in 1 byte units, or in 4KiB pages  
    uint16_t base;     //A 32-bit value containing the linear address where the segment begins.
    uint8_t  base_2;
    uint8_t  access_byte;
    uint8_t flags;     //limit2 and flags combined (each 4 bits as per osdev)
    uint8_t base_3;
}gdt_t[5];

static struct __attribute__((packed)) gdtr_t {
    uint16_t limit;
    uint64_t base;
}gdtr;

void gdt_encode(uint8_t entries, uint32_t limit, uint32_t base, uint8_t acces_byte, uint8_t granuality) {
    //Encode base
    gdt_t[entries].base = (base & 0xFFFF);
    gdt_t[entries].base_2 = (base >> 16) & 0xFF;
    gdt_t[entries].base_3 = (base >> 24) & 0xFF;
    //Encode limit && flags
    gdt_t[entries].limit = (limit & 0xFFFF);
    gdt_t[entries].flags = (limit >> 16) & 0x0F;
    gdt_t[entries].flags |= (granuality & 0xF0);
    //Encode access byte
    gdt_t[entries].access_byte = acces_byte;
}

extern void setGdt(uint16_t limit, uint64_t base);

void set_gdt_entry_descriptor(void) {   //Further, no GDT can have a size of 0 bytes.  
    gdtr.base = (uint64_t)&gdt_t;
    gdtr.limit = (uint16_t)sizeof(gdt_t) - 1;
    gdt_encode(0, 0, 0, 0, 0);                   //NULL descriptor
    gdt_encode(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    //Kernel Mode Code Segment
    gdt_encode(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    //Kernel Mode Data Segment
    gdt_encode(3, 0, 0xFFFFFFFF, 0xF2, 0xCF);    //User Mode Data Segment
    gdt_encode(4, 0, 0xFFFFFFFF, 0xFA, 0xCF);    //User Mode Code Segment
    setGdt(gdtr.limit, gdtr.base);                //gdt entry
}

//_________________________________________________ Interrupt Descriptor Table (IDT/IDTR) _________________________________________________
static struct __attribute__((packed)) InterruptDescriptor64 {
    uint16_t offset;                  // offset bits 0..15 - 64-bit value, split in three parts. It represents the address of the entry point of the Interrupt Service Routine.
    uint16_t segment_selector;        // points to GDT code segment
    uint8_t  ist;                     // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t  type_attributes;         // gate type, dpl, and p fields
    uint16_t offset_2;                // offset bits 16..31
    uint32_t offset_3;                // offset bits 32..63
    uint32_t zero;                    // reserved
}idt_t[256];

static struct __attribute__((packed)) idtr_t {   //Pointer to correct IDT entry
    uint16_t limit;    //One less than the size of the IDT in bytes
    uint64_t base;     //The linear address of the Interrupt Descriptor Table (not the physical address, paging applies).
}idtr;

void idt_encode(uint8_t entries, void* offset, uint8_t type_attributes) {    //must return using iretq instaed of iret
    //Encode offset
    idt_t[entries].offset = ((uint64_t)offset & 0xFFFF);
    idt_t[entries].offset_2 = ((uint64_t)offset >> 16) & 0xFFFF;
    idt_t[entries].offset_3 = ((uint64_t)offset >> 32) & 0xFFFFFFFF;
    //Encode segment_selector
    idt_t[entries].segment_selector = 0x08;
    //Encode ist
    idt_t[entries].ist = 0;
    //Encode type_attributes
    idt_t[entries].type_attributes = type_attributes;

    idt_t[entries].zero = 0;

}

extern void* isr_stub_table[];

void set_idt_entry_descriptor(void) {
    idtr.base = (uint64_t)&idt_t;
    idtr.limit = (uint16_t)sizeof(idt_t) - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_encode(vector, isr_stub_table[vector], 0x8E);
    }
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti");
}
//_________________________________________________ 8259 PIC _________________________________________________

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

static inline uint8_t inb(uint16_t port)    //Receives a 8/16/32-bit value from an I/O location. Traditional names are inb, inw and inl respectively.
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

void PIC_sendEOI(uint8_t irq)   //Wait a very small amount of time (1 to 4 microseconds, generally).
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
	
	outb(PIC1_COMMAND,PIC_EOI);
}

void PIC_remap(int offset1, int offset2)
{
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 1 << CASCADE_IRQ);        // ICW3: tell Master PIC that there is a slave PIC at IRQ2
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	// Unmask both PICs.
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
}

void PIC_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void IRQ_set_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

void IRQ_clear_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}

#define PIC_READ_IRR 0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR 0x0b    /* OCW3 irq service next CMD read */

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}
