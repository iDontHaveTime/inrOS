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
        verbose_scope(print(L"[VERBOSE] Framebuffer size: ");
                      printh64(fb.framebufferSize); newline(););
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
                  print(L"[VERBOSE] Boot info start: ");
                  printh64((uint64_t)kernelImageInfo.bootInfo); newline();
                  print(L"[VERBOSE] Image start: ");
                  printh64((uint64_t)kernelImageInfo.imageStart); newline();
                  print(L"[VERBOSE] Image end: ");
                  printh64((uint64_t)kernelImageInfo.imageEnd); newline();
                  print(L"[VERBOSE] Entry point: ");
                  printh64((uint64_t)kernelImageInfo.entryPoint); newline(););

    verbose_scope(
        verbose(L"Boot mapping info:");
        for(uint8_t i = 0; i < kernelImageInfo.bootInfo->mapCount; i++) {
            const mapping_info* mi = kernelImageInfo.bootInfo->maps + i;
            print(L"  Map ");
            printh8(i);
            print(L":");
            newline();

            print(L"    Physical address: ");
            printh64(mi->paddr);
            newline();

            print(L"    Virtual address: ");
            printh64(mi->vaddr);
            newline();

            print(L"    Page count: ");
            printh32(mi->pageCount);
            newline();

            print(L"    Permissions: ");
            printh8(mi->permissions);
            newline();
        });

    memorymap_info mminfo;
    EFI_STATUS mmStatus = getMemoryMap(&mminfo);
    if(mmStatus != EFI_SUCCESS) {
        return mmStatus;
    }

#define PRINT_MEMORY_MAP
#ifdef PRINT_MEMORY_MAP
    verbose_scope(
        verbose(L"EFI Memory map:");
        uint32_t entries = mminfo.memoryMapSize / mminfo.memoryMapDescSize;

        for(uint32_t i = 0; i < entries; i++) {
            const EFI_MEMORY_DESCRIPTOR* entry =
                (const EFI_MEMORY_DESCRIPTOR*)(((char*)mminfo.memoryMap) +
                                               i * mminfo.memoryMapDescSize);

            if(entry->Type) print(L"  Map ");
            printh32(i);
            print(L":");
            newline();

            print(L"    Address: ");
            printh64((uintptr_t)entry->PhysicalStart);
            newline();
            print(L"    Number of pages: ");
            printh64(entry->NumberOfPages);
            newline();

            print(L"    Memory type: ");
            switch((EFI_MEMORY_TYPE)entry->Type) {
                case EfiReservedMemoryType:
                    println(L"EfiReservedMemoryType");
                    break;
                case EfiLoaderCode:
                    println(L"EfiLoaderCode");
                    break;
                case EfiLoaderData:
                    println(L"EfiLoaderData");
                    break;
                case EfiBootServicesCode:
                    println(L"EfiBootServicesCode");
                    break;
                case EfiBootServicesData:
                    println(L"EfiBootServicesData");
                    break;
                case EfiRuntimeServicesCode:
                    println(L"EfiRuntimeServicesCode");
                    break;
                case EfiRuntimeServicesData:
                    println(L"EfiRuntimeServicesData");
                    break;
                case EfiConventionalMemory:
                    println(L"EfiConventionalMemory");
                    break;
                case EfiUnusableMemory:
                    println(L"EfiUnusableMemory");
                    break;
                case EfiACPIReclaimMemory:
                    println(L"EfiACPIReclaimMemory");
                    break;
                case EfiACPIMemoryNVS:
                    println(L"EfiACPIMemoryNVS");
                    break;
                case EfiMemoryMappedIO:
                    println(L"EfiMemoryMappedIO");
                    break;
                case EfiMemoryMappedIOPortSpace:
                    println(L"EfiMemoryMappedIOPortSpace");
                    break;
                case EfiPalCode:
                    println(L"EfiPalCode");
                    break;
                case EfiPersistentMemory:
                    println(L"EfiPersistentMemory");
                    break;
                case EfiUnacceptedMemoryType:
                    println(L"EfiUnacceptedMemoryType");
                    break;
                case EfiMaxMemoryType:
                    println(L"EfiMaxMemoryType");
                    break;
                default:
                    println(L"Unknown");
                    break;
            }
        }

        mmStatus = getMemoryMap(&mminfo);
        if(mmStatus != EFI_SUCCESS) { return mmStatus; });
#endif

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
    fbToKernelDataStruct(&fb, &kernelImageInfo.bootInfo->kd);
    mmToKernelDataStruct(&mminfo, &kernelImageInfo.bootInfo->kd);

    jumpIntoElf64Image(kernelImageInfo.entryPoint, kernelImageInfo.bootInfo,
                       kernelImageInfo.stack);
}
