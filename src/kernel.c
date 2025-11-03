// main.c
#include <stdint.h>
#include "limine.h"
#include "print.h"


void kmain(void) {
    // Wait until Limine provides framebuffer


    fb_puts(&framebuffer, "KOKOT", 10, 10, 0x00FF00);

    for (;;) __asm__("hlt");
}
