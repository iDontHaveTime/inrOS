#ifndef PAGING_PAGING_H
#define PAGING_PAGING_H

#include <stdint.h>

#include "../../kernel_info.h"

namespace paging {

struct PML4E {
    uint64_t data;
};

struct PDPTE {
    uint64_t data;
};

struct PDE {
    uint64_t data;
};

struct PTE {
    uint64_t data;
};

extern "C" PML4E kernelL4PageTable[512];
PML4E* getPML4EAddrRel(void);

__attribute__((noreturn)) void jump_into_vm_kernel(boot_info* data);

} // namespace paging

#endif // PAGING_PAGING_H
