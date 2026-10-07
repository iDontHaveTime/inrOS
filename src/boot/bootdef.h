#ifndef BOOT_BOOTDEF_H
#define BOOT_BOOTDEF_H

#include <efi.h>

extern EFI_HANDLE ImageHandle;
extern EFI_SYSTEM_TABLE* SystemTable;

typedef struct {
    EFI_PHYSICAL_ADDRESS framebufferAddress;
    UINTN framebufferSize;
    UINT32 horizontalRes, verticalRes, pixelsPerScanLine;
    EFI_GRAPHICS_PIXEL_FORMAT pixelFormat;
} framebuffer_info;

typedef struct {
    UINTN memoryMapSize;
    EFI_MEMORY_DESCRIPTOR* memoryMap;
    UINTN memoryMapKey;
    UINTN memoryMapDescSize;
    UINT32 memoryMapDescVersion;
} memorymap_info;

#endif // BOOT_BOOTDEF_H
