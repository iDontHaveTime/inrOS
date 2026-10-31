#ifndef BOOT_BOOTIO_H
#define BOOT_BOOTIO_H

#include "bootdef.h"

VOID EFIAPI clearscreen(VOID);
VOID EFIAPI print(CHAR16* str);
VOID EFIAPI newline(VOID);
VOID EFIAPI println(CHAR16* str);

VOID EFIAPI printh8(uint8_t i);
VOID EFIAPI printh16(uint16_t i);
VOID EFIAPI printh32(uint32_t i);
VOID EFIAPI printh64(uint64_t i);

#ifndef NDEBUG
static inline VOID EFIAPI _print_verbose(CHAR16* str) {
    print(L"[VERBOSE] ");
    println(str);
}
#define verbose(str) _print_verbose(str)
#define else_vb(str) else verbose(str)
#define verbose_scope(...) \
    do {                   \
        __VA_ARGS__        \
    } while(0)
#else
#define verbose(str) (void)0
#define else_vb(str) (void)0
#define verbose_scope(...) (void)0
#endif

static inline uint32_t EFIAPI getpixel(uint8_t r, uint8_t g, uint8_t b,
                                       EFI_GRAPHICS_PIXEL_FORMAT format) {
    switch(format) {
        case PixelRedGreenBlueReserved8BitPerColor:
            return (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16);
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            return (uint32_t)b | ((uint32_t)g << 8) | ((uint32_t)r << 16);
            break;
        case PixelBitMask:
        case PixelBltOnly:
        case PixelFormatMax:
            return 0;
    }
}

static inline VOID EFIAPI putpixel(unsigned x, unsigned y, uint8_t r, uint8_t g,
                                   uint8_t b, framebuffer_info* fb) {
    *((uint32_t*)(fb->framebufferAddress + 4 * fb->pixelsPerScanLine * y +
                  4 * x)) = getpixel(r, g, b, fb->pixelFormat);
}

static inline VOID EFIAPI fillpixel(unsigned x, unsigned y, unsigned w,
                                    unsigned h, uint8_t r, uint8_t g, uint8_t b,
                                    framebuffer_info* fb) {
    uint32_t pixel = getpixel(r, g, b, fb->pixelFormat);

    for(unsigned i = 0; i < w; i++) {
        for(unsigned j = 0; j < h; j++) {
            *((uint32_t*)(fb->framebufferAddress +
                          4 * fb->pixelsPerScanLine * (y + j) + 4 * (x + i))) =
                pixel;
        }
    }
}

EFI_FILE_HANDLE EFIAPI getThisVolume(VOID);

EFI_FILE_HANDLE EFIAPI openFileRO(EFI_FILE_HANDLE volume, CHAR16* fileName);
VOID EFIAPI closeFile(EFI_FILE_HANDLE file);

UINT64 EFIAPI getFileSize(EFI_FILE_HANDLE file);

char* EFIAPI loadFileIntoMemory(EFI_FILE_HANDLE file, UINTN fileSize);

#endif // BOOT_BOOTIO_H
