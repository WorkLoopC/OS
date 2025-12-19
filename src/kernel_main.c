#include <stdint.h>
#include "print.h"
#include "kernel_memory.h"
#include "limine.h"

void kmain(void) {
    char buf[19];
    int i = 0;
    int x = 8;
    pmm_init(memmap_request.response);
    while (i != 5) {
        fb_puts_char(&framebuffer, buf, 0, x, 0x00FF00);
        print_hex((uintptr_t)pages, buf);
        i++;
        x += 8;
    }
    for (;;) __asm__("hlt");
}