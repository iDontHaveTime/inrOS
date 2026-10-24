#include <stdint.h>

#include "../kernel_info.h"

static inline uint32_t getpixel(uint8_t r, uint8_t g, uint8_t b,
                                uefi_pixel_format format) {
    switch(format) {
        case UEFI_PixelRedGreenBlueReserved8BitPerColor:
            return (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16);
            break;
        case UEFI_PixelBlueGreenRedReserved8BitPerColor:
            return (uint32_t)b | ((uint32_t)g << 8) | ((uint32_t)r << 16);
            break;
        default:
            return 0;
    }
}

[[gnu::used]]
static inline void putpixel(unsigned x, unsigned y, uint8_t r, uint8_t g,
                            uint8_t b, kernel_data::uefi_fb& fb) {
    *((uint32_t*)(fb.framebufferAddress + 4 * fb.pixelsPerScanLine * y +
                  4 * x)) = getpixel(r, g, b, fb.pixelFormat);
}

static inline void fillpixel(unsigned x, unsigned y, unsigned w, unsigned h,
                             uint8_t r, uint8_t g, uint8_t b,
                             kernel_data::uefi_fb& fb) {
    uint32_t pixel = getpixel(r, g, b, fb.pixelFormat);

    for(unsigned i = 0; i < w; i++) {
        for(unsigned j = 0; j < h; j++) {
            *((uint32_t*)(fb.framebufferAddress +
                          4 * fb.pixelsPerScanLine * (y + j) + 4 * (x + i))) =
                pixel;
        }
    }
}

void panic_quiet() {
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

extern "C" void _start(kernel_data* data) {
    if(!data) panic_quiet();

    if(data->ufb.framebufferAddress) {
        fillpixel(0, 0, data->ufb.horizontalRes, data->ufb.verticalRes, 0, 0, 0,
                  data->ufb);
    }

    panic_quiet();
}
