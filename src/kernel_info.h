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
} kernel_data;

#endif // SRC_KERNEL_INFO_H
