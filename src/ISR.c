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
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

inline void exception_handler() {
    __asm__ volatile ("cli; hlt");
}

//_________________________________________________ Interrupt Descriptor Table (IDT/IDTR) _________________________________________________
static struct __attribute__((packed)) InterruptDescriptor64 {
    uint16_t offset;                  // offset bits 0..15 - 64-bit value, split in three parts. It represents the address of the entry point of the Interrupt Service Routine.
    uint16_t segment_selector;        // points to GDT code segment
    uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t  type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;        // offset bits 16..31
    uint32_t offset_3;        // offset bits 32..63
    uint32_t zero;            // reserved
}idt_t[256];

static struct __attribute__((packed)) idtr_t {   //Pointer to correct IDT entry
    uint16_t limit;    //One less than the size of the IDT in bytes
    uint64_t base;     //The linear address of the Interrupt Descriptor Table (not the physical address, paging applies).
}idtr;

void idt_encode(uint8_t entries, uint64_t offset, uint16_t segment_selector, uint8_t ist, uint8_t type_attributes) {    //must return using iretq instaed of iret
    //Encode offset
    gdt_t[entries].offset = (offset & 0xFFFF);
    gdt_t[entries].offset_2 = (offset >> 16) & 0xFFFF
    gdt_t[entries].offset_3 = (offset >> 32) & 0xFFFFFFFF
    //Encode segment_selector
    gdt_t[entries].segment_selector = segment_selector;
    //Encode ist
    gdt_t[entries].ist = 0;
    //Encode type_attributes
    gdt_t[entries].type_attributes = type_attributes;

    gdt_t[entries].zero = 0;

}

void set_idt_entry_descriptor(void) {   
    idtr.base = (uint64_t)&idt_t;           
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti");
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
    gdtr.base = (uint64_t)&gdt_t;             //setting register base to first table element
    gdtr.limit = (sizeof(struct GlobalDescriptor64) * 5) - 1;
    gdt_encode(0, 0, 0, 0, 0);                   //NULL descriptor
    gdt_encode(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    //Kernel Mode Code Segment
    gdt_encode(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    //Kernel Mode Data Segment
    gdt_encode(3, 0, 0xFFFFFFFF, 0xF2, 0xCF);    //User Mode Data Segment
    gdt_encode(4, 0, 0xFFFFFFFF, 0xFA, 0xCF);    //User Mode Code Segment
    setGdt(gdtr.limit, gdtr.base);                //gdt entry
}



