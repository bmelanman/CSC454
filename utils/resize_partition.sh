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

# Target disk
DISK=/dev/vda
# Target partition
PART_NUM=2

sync

# Resize partition
parted "${DISK}" "resizepart" "${PART_NUM}" "100%"

# Resize filesystem
resize2fs "${DISK}${PART_NUM}"

# Print new partition info
parted "${DISK}" "print" "all"

# Done!
echo "Done!"
