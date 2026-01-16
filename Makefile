# Makefile for ParadoxOS

# Compiler and Flags
CC = gcc
LD = ld
CFLAGS = -g -m64 -march=x86-64 -ffreestanding -fno-builtin -nostdlib -mno-red-zone -Wall -Wextra -I src/boot
LDFLAGS = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T src/kernel/linker.ld

# Directories
BUILD_DIR = build
SRC_DIR = src

# Files
KERNEL_SRC = $(shell find $(SRC_DIR) -name "*.c")
KERNEL_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(KERNEL_SRC))
KERNEL_BIN = $(BUILD_DIR)/kernel.elf
ISO_IMAGE = $(BUILD_DIR)/paradoxos.iso

# Limine Version
LIMINE_VERSION = v8.x-binary
LIMINE_Create_Dir = $(BUILD_DIR)/limine

.PHONY: all clean run iso

all: $(ISO_IMAGE)

# 1. Download Limine (If not exists)
$(LIMINE_Create_Dir):
	mkdir -p $(BUILD_DIR)
	git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary-branch --depth=1 $(LIMINE_Create_Dir)
	make -C $(LIMINE_Create_Dir)

# 2. Compile Kernel
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 3. Link Kernel
$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJ)

# 4. Create ISO
iso: $(KERNEL_BIN) $(LIMINE_Create_Dir)
	mkdir -p $(BUILD_DIR)/iso_root
	cp $(KERNEL_BIN) $(BUILD_DIR)/iso_root/
	cp limine.conf $(BUILD_DIR)/iso_root/
	cp $(LIMINE_Create_Dir)/limine-bios.sys $(BUILD_DIR)/iso_root/
	cp $(LIMINE_Create_Dir)/limine-bios-cd.bin $(BUILD_DIR)/iso_root/
	cp $(LIMINE_Create_Dir)/limine-uefi-cd.bin $(BUILD_DIR)/iso_root/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(BUILD_DIR)/iso_root -o $(ISO_IMAGE)
	$(LIMINE_Create_Dir)/limine bios-install $(ISO_IMAGE)

# 5. Run in QEMU
run: iso
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M

clean:
	rm -rf $(BUILD_DIR)
