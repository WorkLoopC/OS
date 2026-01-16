#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H
#include "limine.h"
#include <stdint.h>
#include <stddef.h>

#define PAGE 4096
#define MAX_PAGES 32768
#define BYTE 8
#define CALCULATED_BITMAP_ELEMENTS 65512 

extern volatile struct limine_memmap_request memmap_request;
extern uintptr_t test;
extern uint32_t test_decimal;
volatile extern uintptr_t physical_address;
/*
extern uintptr_t free_pages[MAX_PAGES];
extern size_t free_page_count;
extern uint64_t total_pages;
*/
void pmm_init(struct limine_memmap_response* memmap);
uintptr_t pmm_alloc_page();
#endif