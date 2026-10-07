.POSIX:

# Tools.
CC ::= clang
CXX ::= $(CC)++

CFLAGS ::= -Wall -Wextra -ffreestanding -nostdlib -std=c99

# Build related stuff.
BUILD_FOLDER ::= build

# Boot related stuff.
BOOT_ROOT ::= $(BUILD_FOLDER)/boot
BOOT_FOLDER ::= $(BOOT_ROOT)/EFI/BOOT
BOOT_TARGET ::= $(BOOT_FOLDER)/BOOTX64.EFI

BOOT_SOURCES ::= src/boot/entry.c src/boot/bootstr.c src/boot/bootdef.c src/boot/bootio.c src/boot/bootseq.c

BOOT_CFLAGS ::= -target x86_64-pc-windows-msvc -fuse-ld=lld-link -mno-stack-arg-probe\
				-mgeneral-regs-only -Wl,-entry:_start,-subsystem:efi_application -isystem /usr/include/efi

all: $(BOOT_TARGET)

# Boot rules.
bootloader: $(BOOT_TARGET)

$(BOOT_TARGET): $(BOOT_FOLDER) $(BOOT_SOURCES)
	$(CC) $(CFLAGS) $(BOOT_CFLAGS) $(BOOT_SOURCES) -o $(BOOT_TARGET)

$(BOOT_FOLDER):
	mkdir -p $(BOOT_FOLDER)

# Build rules.
$(BUILD_FOLDER):
	mkdir -p $(BUILD_FOLDER)

# Misc rules.
clean:
	rm -rf $(BOOT_ROOT)
	rm -rf $(BUILD_FOLDER)