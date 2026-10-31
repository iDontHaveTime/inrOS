#ifndef GRAPHICS_GDRIVER_H
#define GRAPHICS_GDRIVER_H

#include "../../kernel_info.h"

/// @brief An abstraction for a graphics driver.
/// @note Very very basic.
struct GraphicsDriver {
    boot_info* bootInfo_;

    using DrawPixelFunction = void (*)(unsigned, unsigned, uint8_t, uint8_t,
                                       uint8_t);
    using FillPixelsFunction = void (*)(unsigned, unsigned, unsigned, unsigned,
                                        uint8_t, uint8_t, uint8_t);

    GraphicsDriver(const GraphicsDriver&) = default;
    GraphicsDriver(GraphicsDriver&&) = default;
    GraphicsDriver& operator=(const GraphicsDriver&) = default;
    GraphicsDriver& operator=(GraphicsDriver&&) = default;

    GraphicsDriver(boot_info* info) : bootInfo_(info) {}

private:
    void UEFIdrawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b);
    void UEFIfillPixels(unsigned x, unsigned y, unsigned w, unsigned h,
                        uint8_t r, uint8_t g, uint8_t b);

public:
    void drawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b) {
        if(hasUEFIGraphics()) {
            UEFIdrawPixel(x, y, r, g, b);
        }
    }

    void fillPixels(unsigned x, unsigned y, unsigned w, unsigned h, uint8_t r,
                    uint8_t g, uint8_t b) {
        if(hasUEFIGraphics()) {
            UEFIfillPixels(x, y, w, h, r, g, b);
        }
    }

    bool bootedFromUEFI() const {
        return bootInfo_->bootedFrom == boot_info::BootSource_UEFI;
    }

    bool hasUEFIGraphics() const {
        return bootedFromUEFI() && bootInfo_->kd.ufb.framebufferAddress;
    }

    unsigned getHorizontalRes(void) const {
        return hasUEFIGraphics() ? bootInfo_->kd.ufb.horizontalRes : 0;
    }

    unsigned getVerticalRes(void) const {
        return hasUEFIGraphics() ? bootInfo_->kd.ufb.verticalRes : 0;
    }
};

#endif // GRAPHICS_GDRIVER_H
