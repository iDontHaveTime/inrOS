#include "../kernel_info.h"
#include "graphics/gdriver.h"
#include "graphics/sbmtext.h"
#include "kernel.h"

extern "C" [[noreturn]] void vm_kernel_start(boot_info* data) {
    GraphicsDriver graphics(data);
    graphics.clearscreen();

    SBMText text(graphics);

    text << "Hello, World!\n";
    panic_quiet();
    // panic_graphics(graphics);
}
