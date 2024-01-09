# Makefile for BinuxOS
# Created by bryc_wall on 12/29/2023

# Target OS parameters
TARGET 	:= BinuxOS
ARCH 	:= x86_64
# DISK_LABEL 	:= $(shell echo $(TARGET) | tr a-z A-Z)

# Directories
SRC_DIR := src
LIB_DIR := lib
BLD_DIR	:= build
FS_MNT	:= build/rootfs

# Source files
LINKER_LD	:= $(SRC_DIR)/linker.ld
GRUB_CFG	:= $(SRC_DIR)/grub.cfg

ASM_SRC	:= $(wildcard $(SRC_DIR)/*.asm)
C_SRC	:= $(wildcard $(SRC_DIR)/*.c)
C_LIB	+= $(wildcard $(LIB_DIR)/*.c)

# Object files
DEPS	:= $(patsubst $(SRC_DIR)/%.asm,$(BLD_DIR)/%.o,$(ASM_SRC)) \
		   $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(C_SRC)) \
		   $(patsubst $(LIB_DIR)/%.c,$(BLD_DIR)/%.o,$(C_LIB))

# Output files
KERNEL 		:= build/kernel-$(TARGET).bin
IMG 		:= build/$(TARGET).img

# Image parameters
IMG_SIZE_MB	:= 64
SECT_LEN	:= 512
GRUB_LEN	:= 2048
PART_OFFSET	:= $(shell echo $$(($(SECT_LEN) * $(GRUB_LEN))))

# Toolchain
CC = $(ARCH)-elf-gcc
OBJCOPY = $(ARCH)-elf-objcopy

# Compiler flags
FLAGS = -nostdlib
INCLUDES = -I$(LIB_DIR)
C_FLAGS = -Wall -Wextra -g -ffreestanding $(INCLUDES)

all: $(KERNEL)

img: $(IMG)

run: $(KERNEL) $(IMG)
	printf "change vnc password\nroot\n" | qemu-system-$(ARCH) -s \
		-monitor stdio \
		-vnc :0,password=on \
		-drive format=raw,file=$(IMG)

test: run

$(IMG): $(KERNEL) $(GRUB_CFG)
	dd if=/dev/zero of=$(IMG) bs=1M count=$(IMG_SIZE_MB)
	parted --script $(IMG) \
		mklabel msdos \
		mkpart primary fat32 $(PART_OFFSET)B 100% \
		set 1 boot on

	losetup /dev/loop0 $(IMG)
	losetup /dev/loop1 $(IMG) -o $(PART_OFFSET)
	mkdosfs -F32 -f 2 /dev/loop1

	mount --mkdir /dev/loop1 $(FS_MNT)

	mkdir -p $(FS_MNT)/boot/grub
	cp $(KERNEL) $(FS_MNT)/boot/kernel.bin
	cp $(GRUB_CFG) $(FS_MNT)/boot/grub/grub.cfg

	grub-install \
		--no-floppy \
		--boot-directory=$(FS_MNT)/boot \
		--directory=/opt/grub-pc/i386-pc/ \
		--modules="normal part_msdos fat ext2 multiboot" \
		/dev/loop0

	umount $(FS_MNT)
	losetup -d /dev/loop0
	losetup -d /dev/loop1

$(KERNEL): $(LINKER_LD) $(DEPS)
	$(CC) -o $@ $(FLAGS) $(INCLUDES) -T $^

$(BLD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BLD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

count:
	wc -l $(ASM_SRC) $(C_SRC) $(C_LIB)

clean:
	@[ -d $(FS_MNT) ] && umount -q -R $(FS_MNT) || true
	@losetup -d /dev/loop0 2>/dev/null || true
	@losetup -d /dev/loop1 2>/dev/null || true
	@rm -rf build || true

.PHONY: all img run test clean $(IMG) $(KERNEL)