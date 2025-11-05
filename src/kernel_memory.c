#include "limine.h"
#include <stdint.h>
#include <stddef.h>



static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

struct limine_memmap_entry* mementry;//memmap->entries[i];





void* kmalloc(size_t size) {
    // find free block in bitmap, mark as used, return pointer
struct limine_memmap_response* memmap = memmap_request.response;
uint64_t entry_count=memmap->entry_count;   //to loop through the entries
uint64_t entries=memmap->entries;           //to access each entry

uint64_t base=mementry->base;       //to know where usable memory starts
uint64_t length=mementry->length;   //to know how big each region is
uint64_t type=mementry->type;       //to skip reserved memory

while(){
    if(== LIMINE_MEMMAP_RESERVED){
        return 0;
    }
    else if( ==LIMINE_MEMMAP_USABLE){

    }
}

}

void kfree(void* ptr) {
    // mark block in bitmap as free

    
}