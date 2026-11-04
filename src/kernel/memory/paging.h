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

// 0b444444444333333333222222222111111111000000000000
constexpr static uintptr_t page_table_mask = 0x1FF000;
constexpr static uintptr_t page_directory_mask = 0x3FE00000;
constexpr static uintptr_t page_directory_ptr_mask = 0x7FC0000000;
constexpr static uintptr_t page_pml4_mask = 0xFF8000000000;
} // namespace paging

#endif // PAGING_PAGING_H
