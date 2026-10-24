# inrOS

## About this repository

This is a hobby kernel project with a custom UEFI bootloader, currently the only supported platform is x86-64 UEFI.

## Status

Currently the bootloader can map the kernel and jump into it.

## Booting

Since this project is (currently) x86-64 UEFI only, and it does not utilize third party bootloaders (like GRUB, or Limine) instead it uses its own bootloader (which can be found under `src/boot`), to boot it you would first build everything via running the Makefile, then have a FAT32 (recommended) formatted USB stick then just copy the `build/boot` folder contents into there, so for example:
```
/dev/sda:
 - EFI/BOOT/BOOTX64.EFI
 - inr-kernel
```
The example above may and probably is not accurate to your machine, it is mainly there to show how the drive should look like inside, not tell you what drive to use.

---

## License

This project is licensed under BSL 1.0.