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
        for (;;) __asm__("hlt");
    }
}

uintptr_t pmm_alloc_page(void) {
    uint64_t page_index = 0;
    bitmap[0] |= 1u;
    for (size_t byte_index = 0; byte_index < bitmap_size; byte_index++) {
        for (uint8_t bit_index = 0; bit_index < BYTE; bit_index++) {
            uint8_t bit = (bitmap[byte_index] >> bit_index) & 1;
            if (bit == 0) { // 0 = un-used page 
                bitmap[byte_index] |= (1 << bit_index);
                page_index = (byte_index * BYTE) + bit_index;
                physical_address = page_index * PAGE;
                if (page_index < total_pages) {
                    //test_decimal = bitmap[byte_index];
                    return physical_address;
                }

            }
            //if bit == 1.. 
        }
    }
    //if (physical_address == 0) for (;;) __asm__("hlt");
}

void pmm_free_page(void) {

    for (size_t i = 0; i < bitmap_size; i++) {

        if ((uintptr_t)bitmap[i] == physical_address) { // Najit adresu a shiftnout na ten bit z 0 na 1 
            for (size_t i = 0; i < bitmap_size; i++) {

            }
            bitmap[i] = 1;

        }
    }
    //for (;;) __asm__("hlt");
}



