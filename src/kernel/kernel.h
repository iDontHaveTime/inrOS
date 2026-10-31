#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include "../kernel_info.h"
#include "graphics/gdriver.h"

__attribute__((noreturn)) void panic_quiet();
__attribute__((noreturn)) void panic_graphics(GraphicsDriver& gd);

__attribute__((noreturn)) extern "C" void vm_kernel_start(boot_info* data);

#endif // KERNEL_KERNEL_H
