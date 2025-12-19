#include "kernel_memory.h"

size_t bitmap_size = 0;
uint32_t bitmap[];
uint64_t pages;

volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

/*
struct limine_memmap_entry {
    uint64_t base;           // physical base address of the memory region
    uint64_t length;        // size of the region
    uint64_t type;          // type of memory (usable, reserved, etc.)
};

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;    // how many memory regions there are
    LIMINE_PTR(struct limine_memmap_entry **) entries;  entries are not guranteed to be sorted !!
};

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    LIMINE_PTR(struct limine_memmap_response *) response;
};
*/
void pmm_init(struct limine_memmap_response* memmap) {
    size_t largest_memory = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* memory_entry = memmap->entries[i];
        size_t current_memory_size = memory_entry->base + memory_entry->length;
        if (current_memory_size > largest_memory) largest_memory = current_memory_size;
    }
    bitmap_size = (largest_memory + 7) / BYTE;  //+7 because of integer division 
    bitmap[bitmap_size];
    //268 435 456 pages(bits)
}

void pmm_pages(struct limine_memmap_response* memmap) {
    uint8_t bit = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* memory_entry = memmap->entries[i];
        for (size_t addr = memory_entry->base; addr < memory_entry->base + memory_entry->length; addr += PAGE)  //; addr += PAGE
        {
            if (memory_entry->type == LIMINE_MEMMAP_USABLE) {
                bitmap[i] |= (1 << bit);
                pages = addr / PAGE;
            }
            else if (memory_entry->type == LIMINE_MEMMAP_RESERVED) {
                bitmap[i] &= ~(1 << i);
            }

            //if (free_page_count > MAX_PAGES - 3) break;
        }
        //if (free_page_count > MAX_PAGES - 3) break;
    }

}

uintptr_t pmm_alloc() {

}

void pmm_free_page(void* addr) {

}


/*
void* kmalloc(size_t size) {
    // find free block in bitmap, mark as used, return pointer


    while () {
        if (== LIMINE_MEMMAP_RESERVED) {
            return 0;
        }
        else if (== LIMINE_MEMMAP_USABLE) {

        }
    }

}
*/
void kfree(void* ptr) {
    // mark block in bitmap as free
}


