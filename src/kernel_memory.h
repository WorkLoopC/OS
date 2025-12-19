#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H
#include "limine.h"
#include <stdint.h>
#include <stddef.h>

#define PAGE 4096
#define MAX_PAGES 32768
#define BYTE 8

extern volatile struct limine_memmap_request memmap_request;
extern uint64_t pages;
extern uintptr_t free_pages[MAX_PAGES];
extern size_t free_page_count;

void pmm_init(struct limine_memmap_response* memmap);
#endif