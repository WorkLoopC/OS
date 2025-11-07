#ifndef KERNEL_MEMORY_H
#define MEMORY_H
#include "limine.h"
#include <stdint.h>
#include <stddef.h>


#define PAGE 4096
uintptr_t free_pages[100];
size_t free_page_count = 0;

//struct limine_memmap_response* memmap = memmap_request.response;
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};
/*
struct limine_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

struct limine_memmap_response {
    uint64_t revision;
    uint64_t entry_count;    // how many memory regions there are
    LIMINE_PTR(struct limine_memmap_entry **) entries;
};

struct limine_memmap_request {
    uint64_t id[4];
    uint64_t revision;
    LIMINE_PTR(struct limine_memmap_response *) response;
};
*/
void* pmm_alloc_page() {
    struct limine_memmap_entry* mementry;
    struct limine_memmap_response* memmap = memmap_request.response;
    int buffer[100];
    for (int i = 0; i < memmap->entry_count; i++) {  //(uint64_t addr = mementry->base; addr < mementry->base + mementry->length; addr += PAGE)
        uint64_t base = mementry->base;      // physical base address of the memory region
        uint64_t length = mementry->length;  // size of the region
        uint64_t type = mementry->type;     // type of memory (usable, reserved, etc.)
        uintptr_t addr = 0;                                                                                //free_pages[free_page_count++] = memmap->entries[i];
        struct limine_memmap_entry* memory_entry = memmap->entries[i];
        for (addr = memory_entry->base; addr < memory_entry->base + memory_entry->length; addr += PAGE)
        {
            free_pages[free_page_count++] = addr;
        }



        //else if (mementry->type == LIMINE_MEMMAP_RESERVED) {}

    }



} // returns physical addr


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

/*
    for (int i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (uintptr_t addr = entry->base; addr < entry->base + entry->length; addr += PAGE) {
                free_pages[free_page_count++] = addr;
            }
        }

        else if (memmap->entries[i] == LIMINE_MEMMAP_RESERVED) {

        }
    }


*/


#endif