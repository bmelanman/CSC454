#!/bin/bash

MAKE_DIR=$(pwd)/Bric_OS/

QEMU_BIN=$(command -v qemu-system-x86_64)

RUN_MAKE=0
START=0
KILL=0

# Check for any CLI arguments
for arg in "$@"; do
	case ${arg} in
	--help | -h)
		echo "Usage: ./start_qemu.sh [OPTION]"
		echo "Start QEMU in the background"
		echo ""
		echo "Options:"
		echo "  --help    Display this help message"

		exit 0
		;;
	--make | -m)
		RUN_MAKE=1
		exit 0
		;;
	--start | -s)
		START=1
		;;
	--kill | -k)
		KILL=1
		exit 0
		;;
	*)
		echo "Invalid argument: ${arg}"
		echo "Try './start_qemu.sh --help' for more information"
		exit 1
		;;
	esac
done

# Run make if specified
if [[ ${RUN_MAKE} -eq 1 ]]; then
	make -C "${MAKE_DIR}" clean all
fi

# Start QEMU if specified
if [[ ${START} -eq 1 ]]; then

	# Make sure QEMU isn't already running
	if pgrep "${QEMU_BIN}"; then
		echo "QEMU is already running"
		exit 1
	fi

	# Run QEMU in the background
	make -C "${MAKE_DIR}" clean run -- QEMU_USER_FLAGS=-S &

	# Done
	exit 0
fi

# Kill QEMU if specified
if [[ ${KILL} -eq 1 ]]; then

	# Make sure QEMU is running
	if ! pgrep "${QEMU_BIN}"; then
		echo "QEMU is not running"
		exit 1
	fi

	# Kill QEMU
	pkill "${QEMU_BIN}"

	# Done
	exit 0
fi
