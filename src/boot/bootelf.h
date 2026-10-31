#ifndef BOOT_BOOTELF_H
#define BOOT_BOOTELF_H

#include <stdint.h>

#include "../kernel_info.h"
#include "bootdef.h"

/// @note This is taken from:
/// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
typedef struct {
    uint8_t emagic[4]; // 0x7f 0x45 0x4c 0x46 (0x7f E L F)
    uint8_t eclass;
    uint8_t edata;
    uint8_t eversionhdr;
    uint8_t eabi;
    uint8_t eabiv;
    uint8_t epad[7];
    uint16_t etype;
    uint16_t emachine;
    uint32_t eversion;
    uint64_t eentry;
    uint64_t ephoff;
    uint64_t eshoff;
    uint32_t eflags;
    uint16_t ehsize;
    uint16_t ephentsize;
    uint16_t ephnum;
    uint16_t eshentsize;
    uint16_t eshnum;
    uint16_t eshstrndx;
} Elf64Header;

_Static_assert(sizeof(Elf64Header) == 64, "ELF64 Header isn't 64 bytes");

typedef struct {
    uint32_t ptype;
    uint32_t pflags;
    uint64_t poffset;
    uint64_t pvaddr;
    uint64_t ppaddr;
    uint64_t pfilesz;
    uint64_t pmemsz;
    uint64_t palign;
} Elf64ProgramHeader;

_Static_assert(sizeof(Elf64ProgramHeader) == 56,
               "ELF64 Program header isn't 56 bytes");

int EFIAPI verifyElfHeaderForx86_64boot(const char* image);

#ifdef __attribute__
#undef __attribute__ // For some reason the uefi headers define it?
#endif

typedef void(__attribute__((sysv_abi, noreturn)) * kernel_start)(boot_info*);

void EFIAPI fbToKernelDataStruct(const framebuffer_info* fb, kernel_data* kd);
void EFIAPI mmToKernelDataStruct(const memorymap_info* mm, kernel_data* kd);

typedef struct {
    char *start, *end;
    char* stack;
    char *imageStart, *imageEnd;
    boot_info* bootInfo;
    kernel_start entryPoint;
} LoadedELF64Image;

int EFIAPI loadElf64Image(const char* image, LoadedELF64Image* info);

__attribute__((noreturn)) void jumpIntoElf64Image(kernel_start entry,
                                                  boot_info* data, char* stack);

#endif // BOOT_BOOTELF_H
