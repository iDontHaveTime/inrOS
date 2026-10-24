#include "bootelf.h"

#include <stdint.h>

#include "bootdef.h"
#include "bootio.h"
#include "bootstr.h"

int verifyElfHeaderForx86_64boot(const char* image) {
    const Elf64Header* hdr = (const Elf64Header*)image;
    int err = 0;

    if(hdr->emagic[0] != 0x7f || hdr->emagic[1] != 'E' ||
       hdr->emagic[2] != 'L' || hdr->emagic[3] != 'F') {
        println(L"[ERROR] ELF magic is not 0x7F E L F");
        err = 1;
    }

    if(hdr->eclass != 2) {
        println(L"[ERROR] ELF is not 64bit");
        err = 1;
    }

    if(hdr->edata != 1) {
        println(L"[ERROR] ELF is not little endian");
        err = 1;
    }

    if(hdr->eabi != 0) {
        println(L"[WARNING] ELF is not SystemV ABI");
    }

    // Rest of the fields depend on these values being correct.
    if(err) return 1;

    if(hdr->etype != 0x02) {
        println(L"[ERROR] ELF type is not executable");
        err = 1;
    }

    if(hdr->emachine != 0x3E) {
        println(L"[ERROR] ELF machine is not x86-64");
        err = 1;
    }

    if(!hdr->eentry) {
        println(L"[ERROR] ELF file has no entry point");
        err = 1;
    }

    if(hdr->ehsize != 64) {
        println(L"[ERROR] ELF header size is not 64 bytes");
        err = 1;
    }

    if(hdr->ephentsize != 0x38) {
        println(L"[ERROR] ELF program header size is not 56 bytes");
        err = 1;
    }

    if(!hdr->ephnum) {
        println(L"[ERROR] ELF has no program header entries");
        err = 1;
    }

    return err;
}

#define PT_LOAD 0x1
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

#define STACK_SIZE 4

int EFIAPI loadElf64Image(const char* image, LoadedELF64Image* info) {
    const Elf64Header* header = (const Elf64Header*)image;
    UINTN nPages = 0;

    const Elf64ProgramHeader* phdr =
        (const Elf64ProgramHeader*)(image + header->ephoff);
    for(uint16_t i = 0; i < header->ephnum; i++) {
        if(phdr[i].ptype != PT_LOAD) continue;
        uint64_t max =
            phdr[i].pmemsz > phdr[i].pfilesz ? phdr[i].pmemsz : phdr[i].pfilesz;
        nPages += (max + (EFI_PAGE_SIZE - 1)) / EFI_PAGE_SIZE;
    }

    nPages += STACK_SIZE; // stack + extra storage
    EFI_PHYSICAL_ADDRESS pages;
    EFI_STATUS status = SystemTable->BootServices->AllocatePages(
        AllocateAnyPages, EfiLoaderData, nPages, &pages);
    if(status != EFI_SUCCESS) {
        println(L"[ERROR] Failed to allocate pages for the ELF image");
        return 1;
    }

    info->start = (char*)pages;
    info->end = info->start + (EFI_PAGE_SIZE * nPages);

    info->stack = info->start + ((STACK_SIZE - 1) * EFI_PAGE_SIZE);
    info->extraStorage = info->stack;

    info->imageStart = info->start + (STACK_SIZE * EFI_PAGE_SIZE);
    info->imageEnd = info->end;

    info->entryPoint = NULL;

    char* pageStart = info->imageStart;
    for(uint16_t i = 0; i < header->ephnum; i++) {
        if(phdr[i].ptype != PT_LOAD) continue;
        uint64_t max =
            phdr[i].pmemsz > phdr[i].pfilesz ? phdr[i].pmemsz : phdr[i].pfilesz;
        unsigned pageCount = (max + (EFI_PAGE_SIZE - 1)) / EFI_PAGE_SIZE;

        char* pageIt = pageStart;
        char* pageEnd = pageIt + (pageCount * EFI_PAGE_SIZE);
        memcpy(pageIt, image + phdr[i].poffset, phdr[i].pfilesz);
        pageIt += phdr[i].pfilesz;
        memset(pageIt, 0, pageEnd - pageIt);

        if(!info->entryPoint && phdr[i].pvaddr < header->eentry) {
            uint64_t vaddr = phdr[i].pvaddr;
            uint64_t vaddrEnd = vaddr + max;
            if(vaddrEnd > header->eentry) {
                info->entryPoint =
                    (kernel_start)(pageStart + (header->eentry - vaddr));
            }
        }

        pageStart = pageEnd;
    }

    if(!info->entryPoint) {
        return 1;
    }

    return 0;
}

void EFIAPI fbToKernelDataStruct(framebuffer_info* fb, kernel_data* kd) {
    kd->ufb.framebufferAddress = (char*)fb->framebufferAddress;
    kd->ufb.framebufferSize = fb->framebufferSize;
    kd->ufb.horizontalRes = fb->horizontalRes;
    kd->ufb.verticalRes = fb->verticalRes;
    kd->ufb.pixelsPerScanLine = fb->pixelsPerScanLine;
    switch(fb->pixelFormat) {
        case PixelRedGreenBlueReserved8BitPerColor:
            kd->ufb.pixelFormat = UEFI_PixelRedGreenBlueReserved8BitPerColor;
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            kd->ufb.pixelFormat = UEFI_PixelBlueGreenRedReserved8BitPerColor;
            break;
        case PixelBitMask:
            kd->ufb.pixelFormat = UEFI_PixelBitMask;
            break;
        case PixelBltOnly:
            kd->ufb.pixelFormat = UEFI_PixelBltOnly;
            break;
        case PixelFormatMax:
            kd->ufb.pixelFormat = UEFI_PixelFormatMax;
            break;
    }
}

__attribute__((noreturn, naked)) void jumpIntoElf64Image(kernel_start entry,
                                                         kernel_data* data,
                                                         char* stack) {
    __asm__ volatile(
        "movq %rdx, %rdi\n"
        "movq %r8, %rsp\n"
        "jmp *%rcx\n");
}
