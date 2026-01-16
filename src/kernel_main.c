#include <stdint.h>
#include "print.h"
#include "kernel_memory.h"
#include "limine.h"

void kmain(void) {
    char buf[10];
    int i = 0;
    pmm_init(memmap_request.response);
    pmm_alloc_page();
    while (i != 5) {
        //print_error(&framebuffer, "error", 0);//0x00FF00
        //print_hex(physical_address, buf);
        //print_hex(pmm_alloc_page(), buf);
        i++;
    }
    print_error(&framebuffer, "error ", 1);

    for (;;) __asm__("hlt");
}