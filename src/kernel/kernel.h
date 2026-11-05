#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include <stddef.h>

#include "../kernel_info.h"
#include "graphics/gdriver.h"

__attribute__((noreturn)) void panic_quiet();
__attribute__((noreturn)) void panic_graphics(GraphicsDriver& gd);

extern "C" {
[[noreturn]] void vm_kernel_start(boot_info* data);
extern uint64_t kernel_page_tables[512];
extern char* bitmap_address;
extern size_t bitmap_byte_count;
}

#endif // KERNEL_KERNEL_H
