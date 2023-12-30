# Created by bryc_wall on 12/29/2023
TARGET 		:= Binux
DISK_LABEL 	:= $(shell echo $(TARGET) | tr a-z A-Z)
ARCH 		:= x86_64

SRC_DIR     := src/arch/$(ARCH)
BUILD_DIR	:= build/arch/$(ARCH)
IMG_DIR		:= build/iso_files

LINKER_LD	:= $(wildcard $(SRC_DIR)/*.ld)
GRUB_CFG	:= $(SRC_DIR)/grub.cfg

ASM_SRC		:= $(wildcard $(SRC_DIR)/*.asm)
C_SRC		:= $(wildcard $(SRC_DIR)/*.c)
ASM_OBJ		:= $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRC))
C_OBJ		:= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))

DEPS 		:= $(ASM_OBJ) $(C_OBJ)
KERNEL 		:= build/kernel-$(TARGET).bin
IMG 		:= build/os-$(TARGET).img
BLK_SIZE	:= 1M
IMG_SIZE_MB	:= 512

CC = $(ARCH)-elf-gcc
OBJCOPY = $(ARCH)-elf-objcopy

FLAGS = -nostdlib
C_FLAGS = -Wall -Wextra -O2 -ffreestanding

all: $(KERNEL)

img: $(IMG)

run: $(KERNEL) $(IMG)
	qemu-system-$(ARCH) \
		-kernel $(KERNEL) \
		-drive format=raw,file=$(IMG) \
		-nographic \
		-append "root=/dev/sda console=ttyAMA0"

test: run

$(IMG): $(KERNEL) $(GRUB_CFG)	
	dd if=/dev/zero of=$(IMG) bs=$(BLK_SIZE) count=$(IMG_SIZE_MB)
	mkfs.fat -F32 -n $(DISK_LABEL) $(IMG)
	#losetup -f -P $(IMG)
	sudo mount --mkdir --target $(IMG_DIR) -o loop $(IMG) --verbose

	sudo mkdir -p $(IMG_DIR)/boot/grub/
	sudo cp $(KERNEL) $(IMG_DIR)/boot/kernel.bin
	sudo cp $(GRUB_CFG) $(IMG_DIR)/boot/grub/grub.cfg

	sudo umount $(IMG_DIR)

$(KERNEL): $(LINKER_LD) $(DEPS)
	$(CC) -o $@ $(FLAGS) -T $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	@[ -d $(IMG_DIR) ] && sudo umount -q $(IMG_DIR) || true
	@rm -rf build

.PHONY: all img run test clean $(IMG) $(KERNEL)