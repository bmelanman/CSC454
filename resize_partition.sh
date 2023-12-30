#!/bin/bash

# Check for sudo
if [[ ${EUID} -ne 0 ]]; then
	echo "Please run as root"
	exit 1
fi

# Check for arguments
if [[ $# -ne 0 ]]; then
	echo "Usage: $0"
	echo "Resize the root partition to fill the disk"
	exit 1
fi

# Check for parted
if ! parted --version; then
	echo "Error: parted is not installed." >&2
	exit 1
fi

# Check for resize2fs
if ! resize2fs --version; then
	echo "Error: resize2fs is not installed." >&2
	exit 1
fi

# Target disk
DISK=/dev/vda
# Target partition
PART_NUM=2

# Resize partition
parted "${DISK}" "resizepart" "${PART_NUM}" "100%"

# Resize filesystem
resize2fs "${DISK}${PART_NUM}"

# Done!
echo "Done!"
df -h /
