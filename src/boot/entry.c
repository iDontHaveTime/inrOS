#include <efi.h>

#include "bootdef.h"
#include "bootio.h"
#include "bootseq.h"

#if !defined(__x86_64__) || !(__SIZEOF_LONG__ == 4) || \
    !(__SIZEOF_WCHAR_T__ == 2)
#error The ABI does not match UEFI's
#endif

EFI_STATUS EFIAPI _start(EFI_HANDLE imgHandleArg_,
                         EFI_SYSTEM_TABLE* sysTableArg_) {
    ImageHandle = imgHandleArg_;
    SystemTable = sysTableArg_;
    clearscreen();

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = getGOP();
    framebuffer_info fb;
    if(!gop) {
        println(L"[WARNING] GOP was not found");
    }
    else fillFBwithGOPinfo(&fb, gop);

    memorymap_info mminfo;
    EFI_STATUS mmStatus = getMemoryMap(&mminfo);
    if(mmStatus != EFI_SUCCESS) {
        return mmStatus;
    }

    EFI_STATUS bsExitStatus = SystemTable->BootServices->ExitBootServices(
        ImageHandle, mminfo.memoryMapKey);

    if(bsExitStatus != EFI_SUCCESS) {
        println(L"[ERROR] ExitBootServices failed");
        return bsExitStatus;
    }

    if(gop) {
        fillpixel(0, 0, 50, 50, 255, 0, 0, &fb);
        fillpixel(50, 0, 50, 50, 0, 255, 0, &fb);
        fillpixel(100, 0, 50, 50, 0, 0, 255, &fb);
    }

    for(;;);
}
