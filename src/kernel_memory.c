#include "kernel_memory.h"

uintptr_t test;
uint32_t test_decimal;

static size_t bitmap_size;
static uint32_t bitmap[CALCULATED_BITMAP_ELEMENTS];
static uint64_t total_pages;
volatile uintptr_t physical_address;

volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

void pmm_init(struct limine_memmap_response* memmap) { //bitmap
    size_t largest_memory = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* memory_entry = memmap->entries[i];
        if (memory_entry->type == LIMINE_MEMMAP_USABLE) {
            size_t current_memory_size = memory_entry->base + memory_entry->length;
            if (current_memory_size > largest_memory) largest_memory = current_memory_size;
        }
    }
    total_pages = largest_memory / PAGE;
    bitmap_size = (total_pages + 7) / BYTE; //+7 because of integer division , [65 512] bytes-524 096 pages 
    if (bitmap_size > CALCULATED_BITMAP_ELEMENTS) {
        //print_error(&framebuffer, "error ", 1);
        //for (;;) __asm__("hlt");
    }
}

uintptr_t pmm_alloc_page(void) {
    size_t page_index = 0;
    bitmap[0] |= 1u;
    for (size_t byte_index = 0; byte_index < bitmap_size; byte_index++) {
        for (uint8_t bit_index = 0; bit_index < BYTE; bit_index++) {
            uint8_t bit = (bitmap[byte_index] >> bit_index) & 1;
            if (bit == 0) { // 0 = un-used page 
                bitmap[byte_index] |= (1 << bit_index);
                page_index = (byte_index * BYTE) + bit_index;
                physical_address = page_index * PAGE;
                if (page_index < total_pages) {
                    return physical_address;
                }
            }
            //if bit == 1.. 
        }
    }
    //if (physical_address == (void*)0)??) for (;;) __asm__("hlt");
}

void pmm_free_page(void) {
    size_t page_index = (size_t)physical_address / PAGE;
    size_t byte_index = page_index / 8;
    size_t bit_index = page_index % 8;
    bitmap[byte_index] &= ~(1 << bit_index);
    //for (;;) __asm__("hlt");
}

static inline uintptr_t read_cr3(void) {    //returns back 48bits, flag bits have to be set to zero
    uintptr_t value;
    __asm__ volatile (
        "mov %%cr3, %0"
        : "=r"(value)
        :
        : "memory"
        );
    return value;
}

void* phys_to_virtual(void* phys_adress) {
    return (void*)phys_adress + 0xFFFF800000000000; //to map the phys_adress into the kernels higher-half and returns 64bits 
}

void* zero(uint64_t* entries) { //zero the page table, setting it to default state
    for (uint16_t i = 0; i < 512; i++) {
        entries[i] = 0;
    }
}

void map_page(uintptr_t virtual_addr, uintptr_t physical_addr, uint64_t flags) {
    uint64_t pt_index = (uint64_t)virtual_addr >> 12 & 0x1FF;
    uint64_t pd_index = (uint64_t)virtual_addr >> 21 & 0x1FF;
    uint64_t pdpt_index = (uint64_t)virtual_addr >> 30 & 0x1FF;
    uint64_t pml4_index = (uint64_t)virtual_addr >> 39 & 0x1FF;

    uintptr_t pml4_phys_adress = read_cr3() & ~0xFFF;   //0xFFF to switch flag bits to zero
    uint64_t* pml4 = phys_to_virtual(pml4_phys_adress);

    if ((pml4[pml4_index] & 1) == 0) {  //entry doesnt exist so i gotta create it
        uintptr_t new_pdpt = pmm_alloc_page() | flags | 0x01;
        zero(phys_to_virtual(new_pdpt));
        pml4[pml4_index] = (uintptr_t)new_pdpt | flags;
    }

    uint64_t* pdpt = phys_to_virtual(pml4[pml4_index] & 0x000FFFFFFFFFF000); //0x000FFFFFFFFFF000 to set flag bits to zero, keep the rest

    if ((pdpt[pdpt_index] & 1) == 0) {
        uintptr_t new_pd = pmm_alloc_page() | flags | 0x01;
        zero(phys_to_virtual(new_pd));
        pdpt[pdpt_index] = (uintptr_t)new_pd | flags;
    }

    uint64_t* pd = phys_to_virtual(pdpt[pdpt_index] & 0x000FFFFFFFFFF000);

    if ((pd[pd_index] & 1) == 0) {
        uintptr_t new_pt = pmm_alloc_page() | flags | 0x01;
        zero(phys_to_virtual(new_pt));
        pd[pd_index] = (uintptr_t)new_pt | flags;
    }

    uint64_t* pt = phys_to_virtual(pd[pd_index] & 0x000FFFFFFFFFF000);

    pt[pt_index] = (physical_addr & 0xFFF) | flags;
    //pt[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01; // Present
}
/*void map_page(void *physaddr, void *virtualaddr, unsigned int flags) {
    // Make sure that both addresses are page-aligned.

    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long *pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.

    unsigned long *pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?

    pt[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01; // Present

    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
Unmapping an entry is essentially the same as above, but instead of assigning the pt[ptindex] a value, you set it to 0x00000000 (i.e. not present).
When the entire page table is empty, you may want to remove it and mark the page directory entry 'not present'.
Of course you don't need the 'flags' or 'physaddr' for unmapping.

*/

void unmap_page(virtual_addr) {

}




