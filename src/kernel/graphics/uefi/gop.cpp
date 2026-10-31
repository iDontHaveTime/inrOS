#include "../gdriver.h"

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

void GraphicsDriver::UEFIdrawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g,
                                   uint8_t b) {
    *((uint32_t*)(bootInfo_->kd.ufb.framebufferAddress +
                  4 * bootInfo_->kd.ufb.pixelsPerScanLine * y + 4 * x)) =
        getpixel(r, g, b, bootInfo_->kd.ufb.pixelFormat);
}

void GraphicsDriver::UEFIfillPixels(unsigned x, unsigned y, unsigned w,
                                    unsigned h, uint8_t r, uint8_t g,
                                    uint8_t b) {
    uint32_t pixel = getpixel(r, g, b, bootInfo_->kd.ufb.pixelFormat);

    for(unsigned i = 0; i < w; i++) {
        for(unsigned j = 0; j < h; j++) {
            *((uint32_t*)(bootInfo_->kd.ufb.framebufferAddress +
                          4 * bootInfo_->kd.ufb.pixelsPerScanLine * (y + j) +
                          4 * (x + i))) = pixel;
        }
    }
}
