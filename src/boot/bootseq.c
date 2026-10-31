#include "bootseq.h"

#include "bootdef.h"
#include "bootio.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL* EFIAPI getGOP(void) {
    EFI_GUID gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    EFI_STATUS gopStatus =
        SystemTable->BootServices->LocateProtocol(&gopGUID, NULL, (void**)&gop);
    if(gopStatus != EFI_SUCCESS) {
        gop = NULL;
    }

    return gop;
}

VOID EFIAPI fillFBwithGOPinfo(framebuffer_info* fb,
                              EFI_GRAPHICS_OUTPUT_PROTOCOL* gop) {
    fb->framebufferAddress = gop->Mode->FrameBufferBase;
    fb->framebufferSize = gop->Mode->FrameBufferSize;
    fb->horizontalRes = gop->Mode->Info->HorizontalResolution;
    fb->verticalRes = gop->Mode->Info->VerticalResolution;
    fb->pixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
    fb->pixelFormat = gop->Mode->Info->PixelFormat;
    switch(fb->pixelFormat) {
        case PixelRedGreenBlueReserved8BitPerColor:
        case PixelBlueGreenRedReserved8BitPerColor:
            break;
        case PixelBitMask:
        case PixelBltOnly:
        case PixelFormatMax:
            println(
                L"[WARNING] GOP has a pixel format unsupported by the "
                L"bootloader");
            break;
    }
}

EFI_STATUS EFIAPI getMemoryMap(memorymap_info* mm) {
    mm->memoryMapSize = 0;
    mm->memoryMap = NULL;
    EFI_STATUS status;

    status = SystemTable->BootServices->GetMemoryMap(
        &mm->memoryMapSize, mm->memoryMap, &mm->memoryMapKey,
        &mm->memoryMapDescSize, &mm->memoryMapDescVersion);
    if(status != EFI_BUFFER_TOO_SMALL) {
        println(
            L"[ERROR] Memory map did not return a status of "
            L"EFI_BUFFER_TOO_SMALL");
        return status;
    }

    status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, mm->memoryMapSize, (void**)&mm->memoryMap);
    if(status != EFI_SUCCESS) {
        println(L"[ERROR] Failed to allocate a memory map");
        return status;
    }

    status = SystemTable->BootServices->GetMemoryMap(
        &mm->memoryMapSize, mm->memoryMap, &mm->memoryMapKey,
        &mm->memoryMapDescSize, &mm->memoryMapDescVersion);

    if(status != EFI_SUCCESS) {
        SystemTable->BootServices->FreePool(mm->memoryMap);
        println(L"[ERROR] Failed to get the memory map");
        return status;
    }

    return status;
}
