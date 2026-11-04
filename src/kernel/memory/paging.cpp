#include "paging.h"

#include <stddef.h>
#include <stdint.h>

#include "../graphics/sbmtext.h"
#include "../kernel.h"
#include "../pic.h"
#include "../std/bitmap.h"

namespace paging {

extern "C" {
alignas(0x1000) PML4E kernelL4PageTable[512];
}

PML4E* getPML4EAddrRel(void) {
    PML4E* addr;
    pic_get_symbol_rip_relative(kernelL4PageTable, addr);
    return addr;
}

struct uefi_map_entry {
    enum : uint32_t {
        EfiReservedMemoryType,      // Unusable.
        EfiLoaderCode,              // Usable.
        EfiLoaderData,              // Technically usable.
        EfiBootServicesCode,        // Usable.
        EfiBootServicesData,        // Usable.
        EfiRuntimeServicesCode,     // Unusable.
        EfiRuntimeServicesData,     // Unusable.
        EfiConventionalMemory,      // Usable.
        EfiUnusableMemory,          // I wonder.
        EfiACPIReclaimMemory,       // Usable when ACPI is enabled.
        EfiACPIMemoryNVS,           // Unusable.
        EfiMemoryMappedIO,          // Unusable.
        EfiMemoryMappedIOPortSpace, // Unusable.
        EfiPalCode,                 // Unusable.
        EfiPersistentMemory,        // Technically usable.
        EfiUnacceptedMemoryType,    // Unusable.
    } type;
    uintptr_t paddr;
    uintptr_t vaddr;
    uint64_t pageCount;
    uint64_t attribute;

    bool alwaysUsableMemory() const {
        switch(type) {
            case EfiLoaderCode:
            case EfiBootServicesCode:
            case EfiBootServicesData:
            case EfiConventionalMemory:
                return true;
            default:
                return false;
        }
    }
};

struct uefi_map_wrapper {
    boot_info* info;

    struct uefi_map_iterator {
        size_t entrySize;
        uefi_map_entry* entry;

        bool operator!=(uefi_map_iterator other) const {
            return entry != other.entry;
        }

        uefi_map_iterator& operator++() {
            entry = (uefi_map_entry*)((char*)entry + entrySize);
            return *this;
        }

        uefi_map_entry* operator*() const {
            return entry;
        }
    };

    uefi_map_iterator begin() const {
        return uefi_map_iterator(info->kd.umm.memoryMapDescSize,
                                 (uefi_map_entry*)info->kd.umm.memoryMap);
    }

    uefi_map_iterator end() const {
        return uefi_map_iterator(
            info->kd.umm.memoryMapDescSize,
            (uefi_map_entry*)((char*)info->kd.umm.memoryMap +
                              info->kd.umm.memoryMapSize));
    }
};

__attribute__((noreturn)) void jump_into_vm_kernel(boot_info* data) {
    size_t pageCount = 0;

    for(const uefi_map_entry* entry : uefi_map_wrapper(data)) {
        pageCount += entry->pageCount;
    }

    for(uefi_map_entry* entry : uefi_map_wrapper(data)) {
        if(!entry->paddr && entry->alwaysUsableMemory()) {
            entry->paddr += 0x1000;
            entry->pageCount--;
            break;
        }
    }

    size_t byteCount = (pageCount + 7) / 8;
    size_t pageAllocCount = (byteCount + 4095) / 4096;

    GraphicsDriver graph(data);
    SBMText text(graph);

    text << pageCount << '\n';
    text << byteCount << '\n';
    text << pageAllocCount << '\n';

    char* bitmapPaddr = nullptr;

    for(const uefi_map_entry* entry : uefi_map_wrapper(data)) {
        if(entry->alwaysUsableMemory() && entry->pageCount >= pageAllocCount) {
            bitmapPaddr = (char*)entry->paddr;
            break;
        }
    }

    if(!bitmapPaddr) {
        panic_graphics(graph);
    }

    text << uintptr_t(bitmapPaddr) << '\n';

    kbitmap memmap(bitmapPaddr, byteCount);
    memmap.setAllBits(true);

    for(const uefi_map_entry* entry : uefi_map_wrapper(data)) {
        if(entry->alwaysUsableMemory()) {
            size_t bit = entry->paddr / 4096;
            for(uint64_t i = 0; i < entry->pageCount; i++) {
                memmap.setBit(bit + i, false);
            }
        }
    }

    for(size_t i = 0; i < pageAllocCount; i++) {
        memmap.setBit((uintptr_t(bitmapPaddr) / 4096) + i, true);
    }

    vm_kernel_start(data);
}

} // namespace paging
