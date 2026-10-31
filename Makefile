.POSIX:

# Tools.
CC ::= clang
CXX ::= $(CC)++

CFLAGS ::= -Wall -Wextra -ffreestanding -nostdlib -std=c11
CXXFLAGS ::= -Wall -Wextra -ffreestanding -nostdlib -std=c++20 -fno-exceptions -fno-rtti

# Build related stuff.
BUILD_FOLDER ::= build

# Boot related stuff.
BOOT_ROOT ::= $(BUILD_FOLDER)/boot
BOOT_FOLDER ::= $(BOOT_ROOT)/EFI/BOOT
BOOT_TARGET ::= $(BOOT_FOLDER)/BOOTX64.EFI

BOOT_SOURCES ::= src/boot/entry.c src/boot/bootstr.c src/boot/bootdef.c src/boot/bootio.c src/boot/bootseq.c src/boot/bootelf.c

BOOT_CFLAGS ::= -target x86_64-pc-windows-msvc -fuse-ld=lld-link -mno-stack-arg-probe\
				-mgeneral-regs-only -Wl,-entry:_start,-subsystem:efi_application -isystem /usr/include/efi

# Kernel related stuff.

KERNEL_TARGET ::= $(BOOT_ROOT)/inr-kernel

KERNEL_SOURCES ::= src/kernel/main.cpp src/kernel/graphics/uefi/gop.cpp src/kernel/memory/paging.cpp\
				   src/kernel/kernel.cpp

KERNEL_CXXFLAGS ::= -target x86_64-unknown-none-elf -mno-red-zone -Wl,-T,src/kernel/kernel_script.ld -mcmodel=small

all: $(KERNEL_TARGET)

# Boot rules.
bootloader: $(BOOT_TARGET)

$(BOOT_TARGET): $(BOOT_FOLDER) $(BOOT_SOURCES)
	$(CC) $(CFLAGS) $(BOOT_CFLAGS) $(BOOT_SOURCES) -o $(BOOT_TARGET)

$(BOOT_FOLDER):
	mkdir -p $(BOOT_FOLDER)

# Kernel rules.
kernel: $(KERNEL_TARGET)

$(KERNEL_TARGET): $(BOOT_TARGET) $(KERNEL_SOURCES)
	$(CXX) $(CXXFLAGS) $(KERNEL_CXXFLAGS) $(KERNEL_SOURCES) -o $(KERNEL_TARGET)

# Build rules.
$(BUILD_FOLDER):
	mkdir -p $(BUILD_FOLDER)

# Misc rules.
clean:
	rm -rf $(BUILD_FOLDER)