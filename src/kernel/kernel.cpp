#include "kernel.h"

#include "graphics/sbmtext.h"

__attribute__((noreturn)) void panic_quiet() {
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

__attribute__((noreturn)) void panic_graphics(GraphicsDriver& g) {
    g.fillPixels(0, 0, g.getHorizontalRes(), g.getVerticalRes(), 0, 0, 255);
    SBMText text(g);

    char kernel_panic_string[sizeof("KERNEL PANIC!\n")];
    kernel_panic_string[0] = 'K';
    kernel_panic_string[1] = 'E';
    kernel_panic_string[2] = 'R';
    kernel_panic_string[3] = 'N';
    kernel_panic_string[4] = 'E';
    kernel_panic_string[5] = 'L';
    kernel_panic_string[6] = ' ';
    kernel_panic_string[7] = 'P';
    kernel_panic_string[8] = 'A';
    kernel_panic_string[9] = 'N';
    kernel_panic_string[10] = 'I';
    kernel_panic_string[11] = 'C';
    kernel_panic_string[12] = '!';
    kernel_panic_string[13] = '\n';
    kernel_panic_string[14] = '\0';

    text << kernel_panic_string;

    panic_quiet();
}
