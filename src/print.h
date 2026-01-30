#ifndef PRINT_H
#define PRINT_H
#include "limine.h"
#include <stdint.h>
#include <stddef.h>

extern struct fb framebuffer;

struct fb {
    uint32_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
};
void print_hex(uintptr_t value, char* buffer);
void print_error(struct fb* fb, const char* str, uint32_t newline);

#endif