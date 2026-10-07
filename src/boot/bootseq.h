#ifndef BOOT_BOOTSEQ_H
#define BOOT_BOOTSEQ_H

#include "bootdef.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL* EFIAPI getGOP(void);
VOID EFIAPI fillFBwithGOPinfo(framebuffer_info* fb,
                              EFI_GRAPHICS_OUTPUT_PROTOCOL* gop);

EFI_STATUS EFIAPI getMemoryMap(memorymap_info* mm);

#endif // BOOT_BOOTSEQ_H
