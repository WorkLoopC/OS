#ifndef KERNEL_MEMORY_H
#define MEMORY_H
#include "limine.h"
#include <stdint.h>
#include <stddef.h>


#define PAGE 4096
uintptr_t free_pages[1000];
size_t free_page_count = 0;


static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};


inline typedef struct node
{
    int val;
    struct node* next;
} node_t;
/*
struct limine_memmap_entry {
    uint64_t base;           // physical base address of the memory region
    uint64_t length;        // size of the region
    uint64_t type;          // type of memory (usable, reserved, etc.)
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
static void* pmm_alloc_page(node_t* head) {
    struct limine_memmap_response* memmap = memmap_request.response;
    node_t* current = head;
    for (int i = 0; i < memmap->entry_count; i++) {  //for (int i = 0; i < memmap->entry_count; i++)
                                                                      
        struct limine_memmap_entry* memory_entry = memmap->entries[i];
        for (uint64_t addr = memory_entry->base; addr < memory_entry->base + memory_entry->length; addr += PAGE)
        {
            if (memory_entry->type != LIMINE_MEMMAP_USABLE) continue;
            current->val=addr;
            current->next->next=NULL;
            free_pages[free_page_count++] = (uintptr_t)current->val;

            //next=current;
            if(free_page_count > 998) break; 
        }
        if(free_page_count > 998) break;
        //else if (mementry->type == LIMINE_MEMMAP_RESERVED) {}

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