#!/bin/bash -e

alias make="make -j\$(nproc)"

# Setup the directory tree
ROOT_DIR="$(pwd)"
TARGET=x86_64-elf
PREFIX="/opt/${TARGET}-cross"
PATH="${PREFIX}/bin:${PATH}"

# Prompt
echo "+------------------------------------------------------------------------------+"
echo "|This script will setup a cross compiler for ${TARGET} on the system.         |"
echo "+------------------------------------------------------------------------------+"

echo "+----------------------------------------------------------+"
echo "|Downloading Binutils and GCC source code...               |"
echo "+----------------------------------------------------------+"

# Download the source code or update it if it already exists
if [[ ! -d binutils-gdb ]]; then
	git clone git://sourceware.org/git/binutils-gdb.git
else
	# Update the source code
	git -C binutils-gdb pull
fi

if [[ ! -d gcc ]]; then
	git clone git://gcc.gnu.org/git/gcc.git
else
	# Update the source code
	git -C gcc pull
fi

echo "+----------------------------------------------------------+"
echo "|Downloading completed! Installation will begin shortly... |"
echo "+----------------------------------------------------------+"

# Create the build directory
mkdir -p build/{binutils-gdb,gcc}

# Build binutils and gdb
cd build/binutils-gdb || exit 1

echo "+----------------------------------------------------------+"
echo "|Configuring Binutils...                                   |"
echo "+----------------------------------------------------------+"
../../binutils-gdb/configure --target="${TARGET}" --prefix="${PREFIX}" --with-sysroot --disable-nls --disable-werror
make

echo "+----------------------------------------------------------+"
echo "|Installing Binutils...                                    |"
echo "+----------------------------------------------------------+"
make install

echo "+----------------------------------------------------------+"
echo "|Successfully installed Binutils!                          |"
echo "+----------------------------------------------------------+"

# Build gcc
cd ../gcc || exit 1

echo "+----------------------------------------------------------+"
echo "|Configuring GCC...                                        |"
echo "+----------------------------------------------------------+"
../../gcc/configure --target="${TARGET}" --prefix="${PREFIX}" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc

echo "+----------------------------------------------------------+"
echo "|Installing GCC...                                         |"
echo "+----------------------------------------------------------+"
make install-gcc
make install-target-libgcc

echo "+----------------------------------------------------------+"
echo "|Successfully installed GCC!                               |"
echo "+----------------------------------------------------------+"

cd "${ROOT_DIR}" || exit 1

# Add the cross compiler to PATH
export PATH="${PREFIX}/bin:${PATH}"

echo "+----------------------------------------------------------+"
echo "|Testing the compiler...                                   |"
echo "+----------------------------------------------------------+"

# Test the compiler
"${TARGET}"-gcc --version

# Done
echo "+----------------------------------------------------------+"
echo "|Testing completed! Binutils and GCC have been installed.  |"
echo "|Don't forget to add ${PREFIX}/bin to your PATH variable!  |"
echo "+----------------------------------------------------------+"
