#ifndef SRC_KERNEL_INFO_H
#define SRC_KERNEL_INFO_H

#include <stdint.h>

enum uefi_pixel_format {
    UEFI_PixelRedGreenBlueReserved8BitPerColor,
    UEFI_PixelBlueGreenRedReserved8BitPerColor,
    UEFI_PixelBitMask,
    UEFI_PixelBltOnly,
    UEFI_PixelFormatMax,
};

typedef struct {
    struct uefi_fb {
        char* framebufferAddress;
        uint64_t framebufferSize;
        uint32_t horizontalRes, verticalRes, pixelsPerScanLine;
        enum uefi_pixel_format pixelFormat;
    } ufb;
    struct uefi_mm {
        uintptr_t memoryMapSize;
        void* memoryMap;
        uint64_t memoryMapKey;
        uintptr_t memoryMapDescSize;
        uint32_t memoryMapDescVersion;
    } umm;
} kernel_data;

#define MAPPING_INFO_EXEC 0x1
#define MAPPING_INFO_WRITE 0x2
#define MAPPING_INFO_READ 0x4

typedef struct {
    uint8_t permissions;
    uint32_t pageCount;
    uintptr_t paddr;
    uintptr_t vaddr;
} mapping_info;

#define MAPPING_INFO_COUNT 96

typedef struct {
    mapping_info maps[MAPPING_INFO_COUNT];
    uint8_t mapCount;
    kernel_data kd;
    enum boot_source {
        BootSource_UEFI,
    } bootedFrom;
    uintptr_t stackAndBootInfoVaddr;
} boot_info;

#endif // SRC_KERNEL_INFO_H
