#include "../kernel_info.h"
#include "graphics/gdriver.h"
#include "graphics/sbmtext.h"
#include "kernel.h"

extern "C" [[noreturn]] void vm_kernel_start(boot_info* data) {
    GraphicsDriver graphics(data);
    graphics.clearscreen();

    SBMText text(graphics);

    text << "Hello, World!\n";

    text << "Bitmap vaddr: " << (const void*)bitmap_address << '\n';
    text << "Bitmap byte count: " << bitmap_byte_count << '\n';

    text << "Kernel Memory Map:\n";

    for(size_t i = 0; i < data->kd.kmm.mapEntries; i++) {
        const auto& entry = data->kd.kmm.memoryMap[i];
        text << " Physical address: " << (const void*)entry.paddr
             << ", Page count: " << entry.pageCount << '\n';
    }

    panic_quiet();
    // panic_graphics(graphics);
}
