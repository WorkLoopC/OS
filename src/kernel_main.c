#include <stdint.h>
#include "print.h"
#include "kernel_memory.h"
#include "limine.h"

void kmain(void) {
    char buf[10];
    int i = 0;
    pmm_init(memmap_request.response);


    uint64_t xx = (uint64_t)physical_address;
    char x[10];
    for (uint8_t i = 0; i < 10; i++) {
        x[i] = (char)xx;
    }
    while (i != 15) {
        print_error(&framebuffer, x, 0);
        print_hex(pmm_alloc_page(), buf);
        pmm_free_page();
        i++;
    }

    for (;;) __asm__("hlt");
}