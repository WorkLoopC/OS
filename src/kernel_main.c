// main.c
#include <stdint.h>
#include "limine.h"
#include "print.h"
#include "kernel_memory.h"

void kmain(void) {
    // Wait until Limine provides framebuffer
    //struct limine_memmap_response* memmap = memmap_request.response;
    //free_pages[0] = memmap->entries[0];
    char buf[19];
    int i = 0;
    int x = 8;
    pmm_alloc_page();
    while (i != 5) {
        itoa_hex(free_pages[i], buf);
        fb_puts_char(&framebuffer, buf, 0, x, 0x00FF00);
        i++;
        x += 8;
    }


    for (;;) __asm__("hlt");
}
/*
itoa_hex(memmap->entries[1], buf);
fb_puts_char(&framebuffer, buf, 0, x, 0x00FF00);
i++;
x += 8;
*/