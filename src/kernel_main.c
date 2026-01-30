#include <stdint.h>
#include "print.h"
#include "kernel_memory.h"
#include "limine.h"

void kmain(void) {
    char buf[10];
    int i = 0;
    pmm_init(memmap_request.response);
    uintptr_t physical_address = pmm_alloc_page();
    void* virt = (void*)0xFFFF900000000000;
    map_page((uintptr_t)virt, physical_address, 0x03);
    *(uint64_t*)virt = 0xDEADBEEF;
    //unmap_page((uintptr_t)virt);
    volatile uint64_t xD = *(volatile uint64_t*)virt;


    while (i != 5) {
        print_error(&framebuffer, buf, 0);
        //print_hex((uintptr_t)xD, buf);
        //print_hex((uintptr_t)virt, buf);
        i++;
    }

    for (;;) __asm__("hlt");
}