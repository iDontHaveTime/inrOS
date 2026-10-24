#include <efi.h>

#include "bootdef.h"
#include "bootelf.h"
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
    else {
        fillFBwithGOPinfo(&fb, gop);
        verbose(L"GOP was located");
    }

    EFI_FILE_HANDLE imageVolume = getThisVolume();
    if(!imageVolume) {
        println(L"[ERROR] Failed to get the image volume");
        return EFI_ABORTED;
    }
    else_vb(L"Image volume fetched");

    EFI_FILE_HANDLE kernelHandle = openFileRO(imageVolume, L"inr-kernel");
    if(!kernelHandle) {
        println(L"[ERROR] Couldn't open kernel file");
        closeFile(imageVolume);
        return EFI_ABORTED;
    }
    else_vb(L"Kernel file loaded");

    UINT64 kernelSize = getFileSize(kernelHandle);
    if(kernelSize == (UINT64)-1) {
        println(L"[ERROR] Couldn't get kernel size");
        closeFile(kernelHandle);
        closeFile(imageVolume);
        return EFI_ABORTED;
    }

    char* kernelImage = loadFileIntoMemory(kernelHandle, kernelSize);
    if(!kernelImage) {
        println(L"[ERROR] Failed to load the kernel into memory");
        closeFile(kernelHandle);
        closeFile(imageVolume);
        return EFI_ABORTED;
    }
    else_vb(L"Kernel image is in memory");

    closeFile(kernelHandle);
    closeFile(imageVolume);

    if(verifyElfHeaderForx86_64boot(kernelImage)) {
        println(L"[ERROR] Verifying kernel image failed");
        SystemTable->BootServices->FreePool(kernelImage);
        return EFI_ABORTED;
    }
    else_vb(L"ELF header is valid");

    LoadedELF64Image kernelImageInfo;
    int kernelAllocationStatus = loadElf64Image(kernelImage, &kernelImageInfo);
    if(kernelAllocationStatus) {
        println(L"[ERROR] Failed to map kernel image");
        SystemTable->BootServices->FreePool(kernelImage);
        return EFI_ABORTED;
    }
    else_vb(L"Kernel was mapped into memory");

    verbose_scope(print(L"[VERBOSE] Pages start: ");
                  printh64((uint64_t)kernelImageInfo.start); newline();
                  print(L"[VERBOSE] Pages end: ");
                  printh64((uint64_t)kernelImageInfo.end); newline();
                  print(L"[VERBOSE] Stack start: ");
                  printh64((uint64_t)kernelImageInfo.stack); newline();
                  print(L"[VERBOSE] Extra storage start: ");
                  printh64((uint64_t)kernelImageInfo.extraStorage); newline();
                  print(L"[VERBOSE] Image start: ");
                  printh64((uint64_t)kernelImageInfo.imageStart); newline();
                  print(L"[VERBOSE] Image end: ");
                  printh64((uint64_t)kernelImageInfo.imageEnd); newline();
                  print(L"[VERBOSE] Entry point: ");
                  printh64((uint64_t)kernelImageInfo.entryPoint); newline(););

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

    kernel_data* kdata = (kernel_data*)kernelImageInfo.extraStorage;
    fbToKernelDataStruct(&fb, kdata);

    jumpIntoElf64Image(kernelImageInfo.entryPoint, kdata,
                       kernelImageInfo.stack);
}
