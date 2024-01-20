# Makefile for BinuxOS
# Created by bryc_wall on 12/29/2023

# Target OS parameters
TARGET 	:= BinuxOS
ARCH 	:= x86_64

# Directories
SRC_DIR := src
LIB_DIR := lib
TST_DIR := test

BLD_DIR	:= build
OBJ_DIR	:= build/objs

TMP_MNT	:= build/temp_mnt
FS_MNT	:= build/rootfs

# Source files
LINKER_LD	:= $(SRC_DIR)/linker.ld
GRUB_CFG	:= $(SRC_DIR)/grub.cfg

ASM_SRC	:= $(wildcard $(SRC_DIR)/*.asm)
C_SRC	:= $(wildcard $(SRC_DIR)/*.c)
C_LIB	:= $(wildcard $(LIB_DIR)/*.c)
C_TST	:= $(wildcard $(TST_DIR)/*.c)

# Object files
DEPS	:= $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.o,$(ASM_SRC)) \
		   $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SRC)) \
		   $(patsubst $(LIB_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_LIB)) \
		   $(patsubst $(TST_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_TST))

# Output files
BLANK_IMG	:= build/blank_grub_fs.img
KERNEL_BIN 	:= build/kernel-$(TARGET).bin
TARGET_IMG 	:= build/$(TARGET).img

# Image parameters
IMG_SIZE_MB	:= 64
SECT_LEN	:= 512
GRUB_LEN	:= 2048
PART_OFFSET	:= $(shell echo $$(($(SECT_LEN) * $(GRUB_LEN))))

# Toolchain
CC 			:= $(ARCH)-elf-gcc
OBJCOPY 	:= $(ARCH)-elf-objcopy

# Compiler flags
INCLUDES 	:= -I$(LIB_DIR) -I$(TST_DIR)
LD_FLAGS 	:= -nostdlib $(INCLUDES)
CC_FLAGS 	:= -Wall -Wextra -g -ffreestanding $(LD_FLAGS)

all: $(KERNEL_BIN)

img: $(TARGET_IMG)

test: run

#qemu-system-$(ARCH) -s \
#	-nographic \
#	-serial mon:stdio \
#	-append 'console=ttyS0' \
#	-drive format=raw,file=$(TARGET_IMG)
run: clean $(TARGET_IMG)
	printf "change vnc password\nroot\n" | qemu-system-$(ARCH) -s \
		-monitor stdio \
		-vnc :0,password=on \
		-drive format=raw,file=$(TARGET_IMG)

$(TARGET_IMG): $(BLANK_IMG) $(KERNEL_BIN) $(GRUB_CFG)
	cp $(BLANK_IMG) $@

	losetup /dev/loop0 $@ -o $(PART_OFFSET)
	mount --mkdir /dev/loop0 $(FS_MNT)

	mkdir -p $(FS_MNT)/boot/grub

	cp $(KERNEL_BIN) $(FS_MNT)/boot/kernel.bin
	cp $(GRUB_CFG) $(FS_MNT)/boot/grub/grub.cfg

	umount $(FS_MNT)
	losetup -d /dev/loop0

$(BLANK_IMG):
	@mkdir -p $(shell dirname $@)

	dd if=/dev/zero of=$@ bs=1M count=$(IMG_SIZE_MB)
	parted --script $@ \
		mklabel msdos \
		mkpart primary fat32 $(PART_OFFSET)B 100% \
		set 1 boot on

	losetup /dev/loop0 $@
	losetup /dev/loop1 $@ -o $(PART_OFFSET)
	mkdosfs -F32 -f 2 /dev/loop1

	mount --mkdir /dev/loop1 $(TMP_MNT)

	grub-install \
		--no-floppy \
		--boot-directory=$(TMP_MNT)/boot \
		--directory=/opt/grub-pc/i386-pc/ \
		--modules="normal part_msdos fat ext2 multiboot" \
		/dev/loop0

	umount $(TMP_MNT)
	rmdir $(TMP_MNT)
	losetup -d /dev/loop0
	losetup -d /dev/loop1

$(KERNEL_BIN): $(LINKER_LD) $(DEPS)
	@mkdir -p $(shell dirname $@)
	$(CC) -o $@ $(LD_FLAGS) -T $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TST_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CC_FLAGS) -c $< -o $@

count:
	wc -l $(ASM_SRC) $(C_SRC) $(C_LIB)

clean-all: clean
	@rm -rf $(BLD_DIR)

clean:
	@[ -d $(FS_MNT) ] && umount -q -R $(FS_MNT) || true
	@losetup -d /dev/loop0 2>/dev/null || true
	@losetup -d /dev/loop1 2>/dev/null || true
	@rmdir $(FS_MNT) 2>/dev/null || true
	@rm -rf $(OBJ_DIR) $(KERNEL_BIN) $(TARGET_IMG)

.PHONY: all img run test clean clean-all count