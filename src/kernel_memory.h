#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H
#include "limine.h"
#include <stdint.h>
#include <stddef.h>
#include "print.h"

extern volatile struct limine_memmap_request memmap_request;

void pmm_init(struct limine_memmap_response* memmap);
void map_page(uintptr_t virtual_addr, uintptr_t physical_addr, uint64_t flags);
void unmap_page(uintptr_t virtual_addr);
void* kmalloc(size_t size);
void free(void* ptr);
#endif