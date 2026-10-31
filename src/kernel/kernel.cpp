#include "kernel.h"

__attribute__((noreturn)) void panic_quiet() {
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

__attribute__((noreturn)) void panic_graphics(GraphicsDriver& g) {
    g.fillPixels(0, 0, g.getHorizontalRes(), g.getVerticalRes(), 0, 0, 255);
    panic_quiet();
}
