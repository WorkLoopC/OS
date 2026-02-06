#include <stdint.h>
#include "print.h"
#include "kernel_memory.h"
#include "limine.h"
#include "ISR.h"

void kernel_init() {
    pmm_init(memmap_request.response);
}

void kmain(void) {
    void kernel_init();

    for (;;) __asm__("hlt");
}

