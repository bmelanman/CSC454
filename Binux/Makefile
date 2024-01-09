# Created by bryc_wall on 12/29/2023
TARGET 		:= BinuxOS
DISK_LABEL 	:= $(shell echo $(TARGET) | tr a-z A-Z)
ARCH 		:= x86_64

SRC_DIR     := src
LIB_DIR     := lib
BUILD_DIR	:= build
FS_MNT		:= build/rootfs

LINKER_LD	:= $(SRC_DIR)/linker.ld
GRUB_CFG	:= $(SRC_DIR)/grub.cfg

ASM_SRC		:= $(wildcard $(SRC_DIR)/*.asm)
C_SRC		:= $(wildcard $(SRC_DIR)/*.c)
C_LIB		+= $(wildcard $(LIB_DIR)/*.c)

ASM_OBJ		:= $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRC))
C_OBJ		:= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))
C_OBJ		+= $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_LIB))

DEPS 		:= $(ASM_OBJ) $(C_OBJ)
KERNEL 		:= build/kernel-$(TARGET).bin
IMG 		:= build/$(TARGET).img

BLK_SIZE	:= 1M
IMG_BLK_LEN	:= 512

CC = $(ARCH)-elf-gcc
OBJCOPY = $(ARCH)-elf-objcopy

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
	dd if=/dev/zero of=$(IMG) bs=$(BLK_SIZE) count=$(IMG_BLK_LEN)
	parted --script $(IMG) \
		mklabel msdos \
		mkpart primary fat32 2048s 100% \
		set 1 boot on

	losetup /dev/loop0 $(IMG)
	losetup /dev/loop1 $(IMG) -o 1048576  # 2048 * 512
	mkfs.vfat -F32 -f 2 /dev/loop1
	@#kpartx -a /dev/loop0
	@#mkfs.vfat -F32 -f 2 /dev/mapper/loop0p1

	@#mount --mkdir /dev/mapper/loop0p1 $(FS_MNT)
	mount --mkdir /dev/loop1 $(FS_MNT)
	@#mkdir -p $(FS_MNT)/boot/grub

	grub-install \
		--no-floppy \
		--root-directory=$(FS_MNT) \
		--directory=/opt/grub-pc/i386-pc/ \
		--modules="normal part_msdos ext2 multiboot" \
		/dev/loop0

	cp $(KERNEL) $(FS_MNT)/boot/kernel.bin
	cp $(GRUB_CFG) $(FS_MNT)/boot/grub/grub.cfg

	#umount -R $(FS_MNT)
	#kpartx -d /dev/loop0
	#losetup -d /dev/loop0

$(KERNEL): $(LINKER_LD) $(DEPS)
	$(CC) -o $@ $(FLAGS) $(INCLUDES) -T $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

count:
	wc -l $(ASM_SRC) $(C_SRC) $(C_LIB)

clean:
	@[ -d $(FS_MNT) ] && umount -q -R $(FS_MNT) || true
	@kpartx -d /dev/loop0 || true
	@losetup -d /dev/loop0 2>/dev/null || true
	@rm -rf build || true

.PHONY: all img run test clean $(IMG) $(KERNEL)