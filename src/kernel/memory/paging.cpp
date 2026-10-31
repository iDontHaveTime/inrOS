#include "paging.h"

#include "../kernel.h"
#include "../pic.h"

namespace paging {

extern "C" {
alignas(0x1000) PML4E kernelL4PageTable[512];
}

PML4E* getPML4EAddrRel(void) {
    PML4E* addr;
    pic_get_symbol_rip_relative(kernelL4PageTable, addr);
    return addr;
}

__attribute__((noreturn)) void jump_into_vm_kernel(boot_info* data) {
    vm_kernel_start(data);
}

} // namespace paging
