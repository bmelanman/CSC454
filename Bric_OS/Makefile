# Makefile for Bric_OS
# Created by bryc_wall on 12/29/2023

SYSTEM_LOG 	:= ../misc/qemu-system-x86_64.log
SERIAL_LOG 	:= ../misc/serial.log
SERIAL_PIPE := ../misc/serial_pipe

# Target OS parameters
TARGET 	:= Bric_OS
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
LINK_LD	:= $(shell find $(SRC_DIR) -type f -name '*.ld')
GRB_CFG	:= $(shell find $(SRC_DIR) -type f -name 'grub.cfg')

# Get all C and ASM files as object files
SOURCES	:= $(shell find $(SRC_DIR) -type f -name '*.c' -o -name '*.asm' ) \
		   $(shell find $(LIB_DIR) -type f -name '*.c') \
		   $(shell find $(TST_DIR) -type f -name '*.c')

OBJECTS	:= $(addprefix $(OBJ_DIR)/,$(SOURCES:=.o))

# Output files
BLANK_IMG	:= $(BLD_DIR)/blank_grub_fs.img
KERNEL_BIN 	:= $(BLD_DIR)/kernel-$(TARGET).bin
TARGET_IMG 	:= $(BLD_DIR)/$(TARGET).img

# Image parameters
IMG_SIZE_MB	:= 64
SECT_LEN	:= 512
GRUB_LEN	:= 2048
PART_OFFSET	:= $(shell echo $$(($(SECT_LEN) * $(GRUB_LEN))))

# Toolchain
PREFIX		:= $(ARCH)-elf-
CC 			:= $(PREFIX)gcc
LD 			:= $(PREFIX)ld
OBJCOPY 	:= $(PREFIX)objcopy
ASM_CC		:= nasm
ASM_FLAGS	:= -f elf64 -g

# Compiler flags
SRC_INCS	:= $(shell find $(SRC_DIR) -type d -printf "-I%p " | sed 's/.$$//')
INCLUDES 	:= -I$(LIB_DIR) -I$(TST_DIR) $(SRC_INCS)
LD_FLAGS 	:= -nostdlib $(INCLUDES)
CC_FLAGS 	:= $(LD_FLAGS) -Wall -Wextra -g -ffreestanding -mno-red-zone

# QEMU flags
QEMU_FLAGS	:= -s -d int,in_asm -D $(SYSTEM_LOG) -no-reboot $(QEMU_USER_FLAGS)
QEMU_DISPLY	:= -display none
QEMU_SERIAL	:= -chardev pipe,id=serial,path=$(SERIAL_PIPE),logfile=$(SERIAL_LOG) -serial chardev:serial
QEMU_DRIVE	:= -drive format=raw,file=$(TARGET_IMG)

all: $(KERNEL_BIN)

img: $(TARGET_IMG)

test:
	@echo ''
	@echo 'SOURCES:'
	@for src in $(SOURCES); do echo $$src; done
	@echo 'OBJECTS:'
	@for obj in $(OBJECTS); do echo $$obj; done

run: clean mkpipes $(TARGET_IMG)
	qemu-system-$(ARCH) $(QEMU_FLAGS) $(QEMU_DISPLY) $(QEMU_SERIAL) $(QEMU_DRIVE)
	@echo ""

mkpipes:
	@[ -p $(SERIAL_PIPE).in ] || mkfifo $(SERIAL_PIPE).in $(SERIAL_PIPE).out

debug: QEMU_FLAGS += -S
debug: run

$(TARGET_IMG): $(BLANK_IMG) $(KERNEL_BIN) $(GRB_CFG)
	cp $(BLANK_IMG) $@

	losetup /dev/loop0 $@ -o $(PART_OFFSET)
	mount --mkdir /dev/loop0 $(FS_MNT)

	mkdir -p $(FS_MNT)/boot/grub

	cp $(KERNEL_BIN) $(FS_MNT)/boot/kernel.bin
	cp $(GRB_CFG) $(FS_MNT)/boot/grub/grub.cfg

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

# Linker target for linking all object files
$(KERNEL_BIN): $(LINK_LD) $(OBJECTS)
	@mkdir -p $(shell dirname $@)
	$(LD) -o $@ $(LD_FLAGS) -T $^

# Generic compiler target for compiling C and ASM files
$(OBJ_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	$(ASM_CC) $(ASM_FLAGS) $< -o $@

count:
	wc -l $(SOURCES)

clean-all: clean
	@rm -rf $(BLD_DIR)

clean:
	@[ -d $(FS_MNT) ] && umount -q -R $(FS_MNT) || true
	@losetup -d /dev/loop0 2>/dev/null || true
	@losetup -d /dev/loop1 2>/dev/null || true
	@rmdir $(FS_MNT) 2>/dev/null || true
	@rm -rf $(OBJ_DIR) $(KERNEL_BIN) $(TARGET_IMG)

.PHONY: all img run test clean clean-all count debug