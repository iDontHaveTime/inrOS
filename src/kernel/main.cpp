#include "../kernel_info.h"
#include "graphics/gdriver.h"
#include "graphics/sbmtext.h"
#include "kernel.h"
#include "memory/paging.h"

extern "C" void vm_kernel_start(boot_info* data) {
    GraphicsDriver graphics(data);
    SBMText text(graphics);
    text.put('\n').put('\n').put('\n').put('\n');

    text << 5318008 << '\n';
    panic_quiet();
    // panic_graphics(graphics);
}

/// When this function is called the processor should be in this state (similar,
/// if not the same as UEFI specifies): Long mode, Paging enabled and identity
/// mapped, Direction in EFLAGS is cleared, FP cw is 0x037F, Interrupts are
/// enabled, although none are supported, CR0.EM and CR0.TS are zero.
extern "C" int /* Use int so the compiler can warn if there's no return (or halt
                  in this case) */
_start(boot_info* data) {
    if(!data) panic_quiet();

    paging::jump_into_vm_kernel(data);
}
