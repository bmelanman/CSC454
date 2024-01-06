# Created by bryc_wall on 12/29/2023
TARGET 		:= BinuxOS
DISK_LABEL 	:= $(shell echo $(TARGET) | tr a-z A-Z)
ARCH 		:= x86_64

SRC_DIR     := src/arch/$(ARCH)
BUILD_DIR	:= build/arch/$(ARCH)
FS_MNT		:= build/rootfs

LINKER_LD	:= $(wildcard $(SRC_DIR)/*.ld)
GRUB_CFG	:= $(SRC_DIR)/grub.cfg

ASM_SRC		:= $(wildcard $(SRC_DIR)/*.asm)
C_SRC		:= $(wildcard $(SRC_DIR)/*.c)
ASM_OBJ		:= $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRC))
C_OBJ		:= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

DEPS 		:= $(ASM_OBJ) $(C_OBJ)
KERNEL 		:= build/kernel-$(TARGET).bin
IMG 		:= build/$(TARGET).img

BLK_SIZE	:= 1M
IMG_SIZE_MB	:= 1024
BOOT_SIZE_MB:= 512

CC = $(ARCH)-elf-gcc
OBJCOPY = $(ARCH)-elf-objcopy

FLAGS = -nostdlib
C_FLAGS = -Wall -Wextra -O2 -ffreestanding

all: $(KERNEL)

img: $(IMG)

run: $(KERNEL) $(IMG)
	 printf "change vnc password\nroot\n" | qemu-system-$(ARCH) \
		-monitor stdio \
		-vnc :0,password=on \
		-drive format=raw,file=$(IMG) \
		#-kernel $(KERNEL) \
		#-cdrom $(IMG) \

test: run

$(IMG): $(KERNEL) $(GRUB_CFG)
	dd if=/dev/zero of=$(IMG) bs=$(BLK_SIZE) count=$(IMG_SIZE_MB)
	parted --script $(IMG) \
		mklabel gpt \
		mkpart primary fat32 1 $(BOOT_SIZE_MB)M \
		set 1 bios_grub on \
		mkpart primary ext2 $(BOOT_SIZE_MB)M 100%

	losetup /dev/loop0 $(IMG)
	kpartx -a /dev/loop0
	mkfs.vfat -F 32 /dev/mapper/loop0p1
	mkfs.ext2 /dev/mapper/loop0p2

	mount --mkdir /dev/mapper/loop0p2 $(FS_MNT)
	#mount --mkdir /dev/mapper/loop0p1 $(FS_MNT)/boot

	mkdir -p $(FS_MNT)/boot/grub

	grub-install \
		--no-floppy \
		--target=i386-pc \
		--bootloader-id=GRUB \
		--boot-directory=$(FS_MNT)/boot \
		--directory=/opt/grub-pc/i386-pc/ \
		--modules="normal part_msdos ext2 multiboot" \
		/dev/loop0

	cp $(KERNEL) $(FS_MNT)/boot/kernel.bin
	cp $(GRUB_CFG) $(FS_MNT)/boot/grub/grub.cfg

	umount -R $(FS_MNT)
	kpartx -d /dev/loop0
	losetup -d /dev/loop0

$(KERNEL): $(LINKER_LD) $(DEPS)
	$(CC) -o $@ $(FLAGS) -T $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	@[ -d $(FS_MNT) ] && umount -q -R $(FS_MNT) || true
	@kpartx -d /dev/loop0 || true
	@losetup -d /dev/loop0 2>/dev/null || true
	@rm -rf build || true

.PHONY: all img run test clean $(IMG) $(KERNEL)