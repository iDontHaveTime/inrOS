#include <stddef.h>
#include <stdint.h>

#include "../../kernel_info.h"
#include "../std/bitmap.h"
#include "../std/string.h"

#define pic_get_symbol_rip_relative(symbolName, variable) \
    asm("leaq " #symbolName "(%%rip), %0\n" : "=r"(variable))

static inline void wrmsr(uint64_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline uint64_t rdmsr(uint64_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

[[noreturn]] static inline void halt() {
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

[[noreturn]] static inline void try_halt_graphics(boot_info* data) {
    if(data->bootedFrom == boot_info::BootSource_UEFI) {
        if(data->kd.ufb.framebufferAddress) {
            uint32_t pixel = 0;
            switch(data->kd.ufb.pixelFormat) {
                case UEFI_PixelRedGreenBlueReserved8BitPerColor:
                    pixel = (uint32_t)255 << 16;
                    break;
                case UEFI_PixelBlueGreenRedReserved8BitPerColor:
                    pixel = 255;
                    break;
                default:
                    pixel = 0;
            }

            for(unsigned i = 0; i < data->kd.ufb.horizontalRes; i++) {
                for(unsigned j = 0; j < data->kd.ufb.verticalRes; j++) {
                    *((uint32_t*)(data->kd.ufb.framebufferAddress +
                                  4 * data->kd.ufb.pixelsPerScanLine * j +
                                  4 * i)) = pixel;
                }
            }
        }
    }
    halt();
}

extern "C" {
alignas(0x1000) uint64_t kernel_page_tables[512];
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

#define PAGE_TABLE_MASK 0x1FF000
#define PAGE_DIRECTORY_MASK 0x3FE00000
#define PAGE_DIRECTORY_PTR_MASK 0x7FC0000000
#define PAGE_PML4_MASK 0xFF8000000000

static inline uint64_t* allocate_page(boot_info* data, kbitmap& bitmap) {
    for(const uefi_map_entry* entry : uefi_map_wrapper(data)) {
        if(entry->alwaysUsableMemory()) {
            size_t bit = entry->paddr / 4096;
            for(uint64_t i = 0; i < entry->pageCount; i++) {
                if(!bitmap.getBit(bit + i)) {
                    bitmap.setBit(bit + i, true);
                    return (uint64_t*)(entry->paddr + (i * 0x1000));
                }
            }
        }
    }
    return nullptr;
}

extern "C" [[noreturn]] void vm_kernel_start(boot_info*);

// rdi rsi rdx rcx r8 r9
[[noreturn, gnu::naked]]
static inline void transfer_to_vm_start(uintptr_t vaddrBootInfoAndStack,
                                        uint64_t* pageTable) {
    asm volatile(
        R"( movabsq $vm_kernel_start, %rdx
    movq %rdi, %rsp
    movq %rsi, %cr3
    jmp *%rdx)");
}

static inline void map_paddr_to_vaddr(boot_info* data, kbitmap& bitmap,
                                      uint64_t* table, uintptr_t vaddr,
                                      uint64_t paddr, uint8_t perm) {
    unsigned pml4_off = (vaddr & PAGE_PML4_MASK) >> 39;

    uint64_t& pml4_e = table[pml4_off];
    if(!(pml4_e & 1)) {
        uint64_t* dirptr_page = allocate_page(data, bitmap);
        if(!dirptr_page) {
            halt();
        }
        memset(dirptr_page, 0, 0x1000);

        pml4_e |= (uintptr_t(dirptr_page) & ~uintptr_t(0xFFF)) | 0x3;
    }

    unsigned dir_ptr_off = (vaddr & PAGE_DIRECTORY_PTR_MASK) >> 30;
    uint64_t& dir_ptr_e =
        ((uint64_t*)(pml4_e & ~uintptr_t(0xFFF)))[dir_ptr_off];
    if(!(dir_ptr_e & 1)) {
        uint64_t* dir_page = allocate_page(data, bitmap);
        if(!dir_page) {
            halt();
        }
        memset(dir_page, 0, 0x1000);

        dir_ptr_e |= (uintptr_t(dir_page) & ~uintptr_t(0xFFF)) | 0x3;
    }

    unsigned dir_off = (vaddr & PAGE_DIRECTORY_MASK) >> 21;
    uint64_t& dir_e = ((uint64_t*)(dir_ptr_e & ~uintptr_t(0xFFF)))[dir_off];
    if(!(dir_e & 1)) {
        uint64_t* pt_page = allocate_page(data, bitmap);
        if(!pt_page) {
            halt();
        }
        memset(pt_page, 0, 0x1000);

        dir_e |= (uintptr_t(pt_page) & ~uintptr_t(0xFFF)) | 0x3;
    }

    unsigned pt_off = (vaddr & PAGE_TABLE_MASK) >> 12;
    uint64_t& pt_e = ((uint64_t*)(dir_e & ~uintptr_t(0xFFF)))[pt_off];
    if(pt_e & 1) {
        halt(); // Page already mapped?
    }

    pt_e |= 0x1;
    if(perm & MAPPING_INFO_WRITE) {
        pt_e |= 0x2;
    }

    pt_e |= (uintptr_t(paddr) & ~uintptr_t(0xFFF));

    if(!(perm & MAPPING_INFO_EXEC)) {
        pt_e |= 1ULL << 63;
    }
}

static inline void map_mapping_to_page_table(boot_info* data, kbitmap& bitmap,
                                             uint64_t* table) {
    for(unsigned i = 0; i < data->mapCount; i++) {
        const mapping_info& info = data->maps[i];
        for(uint64_t j = 0; j < info.pageCount; j++) {
            map_paddr_to_vaddr(data, bitmap, table, info.vaddr + (j * 0x1000),
                               info.paddr + (j * 0x1000), info.permissions);
            if(info.permissions & MAPPING_INFO_EXEC) {
                map_paddr_to_vaddr(data, bitmap, table,
                                   info.paddr + (j * 0x1000),
                                   info.paddr + (j * 0x1000), info.permissions);
            }
        }
    }
}

static inline size_t countEFIPages(boot_info* data) {
    size_t pageCount = 0;
    for(const uefi_map_entry* entry : uefi_map_wrapper(data)) {
        pageCount += entry->pageCount;
    }
    return pageCount;
}

static inline void reserveEFIFirst0x0Page(boot_info* data) {
    for(uefi_map_entry* entry : uefi_map_wrapper(data)) {
        if(!entry->paddr && entry->alwaysUsableMemory()) {
            entry->paddr += 0x1000;
            entry->pageCount--;
            break;
        }
    }
}

static inline char* findEFIBitmapPaddr(boot_info* data, size_t pageAllocCount) {
    char* bitmapPaddr = nullptr;
    for(const uefi_map_entry* entry : uefi_map_wrapper(data)) {
        if(entry->alwaysUsableMemory() && entry->pageCount >= pageAllocCount) {
            bitmapPaddr = (char*)entry->paddr;
            break;
        }
    }
    return bitmapPaddr;
}

static inline void addBitmapToMapping(boot_info* data, char* bitmapPaddr,
                                      size_t pageAllocCount) {
    mapping_info* bitmapMap = &data->maps[data->mapCount++];
    bitmapMap->paddr = uintptr_t(bitmapPaddr);
    bitmapMap->pageCount = pageAllocCount;
    bitmapMap->permissions = MAPPING_INFO_READ | MAPPING_INFO_WRITE;

    data->minVaddr -= pageAllocCount * 0x1000;
    bitmapMap->vaddr = data->minVaddr;
}

static inline void addFramebufferToMapping(boot_info* data) {
    if(data->bootedFrom == boot_info::BootSource_UEFI &&
       data->kd.ufb.framebufferAddress) {
        size_t fbPages = (data->kd.ufb.framebufferSize + 4095) / 4096;
        mapping_info* fbMap = &data->maps[data->mapCount++];
        fbMap->paddr = uintptr_t(data->kd.ufb.framebufferAddress);
        fbMap->pageCount = fbPages;
        fbMap->permissions = MAPPING_INFO_READ | MAPPING_INFO_WRITE;

        data->minVaddr -= fbPages * 0x1000;
        fbMap->vaddr = data->minVaddr;

        data->kd.ufb.framebufferAddress = (char*)fbMap->vaddr;
    }
}

/// When this function is called the processor should be in this state (similar,
/// if not the same as UEFI specifies): Long mode, Paging enabled and identity
/// mapped, Direction in EFLAGS is cleared, FP cw is 0x037F, Interrupts are
/// enabled, although none are supported, CR0.EM and CR0.TS are zero.
extern "C" [[noreturn]] void _start(boot_info* data) {
    if(!data) halt();
    uint64_t efer_val = rdmsr(0xC0000080);
    efer_val |= 1 << 11;
    wrmsr(0xC0000080, efer_val);

    size_t pageCount = countEFIPages(data);
    reserveEFIFirst0x0Page(data);

    size_t byteCount = (pageCount + 7) / 8;
    size_t pageAllocCount = (byteCount + 4095) / 4096;

    char* bitmapPaddr = findEFIBitmapPaddr(data, pageAllocCount);
    if(!bitmapPaddr) {
        try_halt_graphics(data);
    }

    if(data->mapCount + 1 +
           unsigned(data->bootedFrom == boot_info::BootSource_UEFI &&
                    data->kd.ufb.framebufferAddress) >=
       MAPPING_INFO_COUNT) {
        try_halt_graphics(data);
    }

    addBitmapToMapping(data, bitmapPaddr, pageAllocCount);
    addFramebufferToMapping(data);

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

    uint64_t* kernelTable;
    pic_get_symbol_rip_relative(kernel_page_tables, kernelTable);

    memset(kernelTable, 0, sizeof(kernel_page_tables));

    map_mapping_to_page_table(data, memmap, kernelTable);
    transfer_to_vm_start(data->stackAndBootInfoVaddr, kernelTable);
}
