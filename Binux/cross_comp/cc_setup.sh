#!/bin/bash -ex

alias make="make -j\$(nproc)"

# Install necessary packages, but dont reinstall them if they are already installed
#PACKAGES=(
#	"base-devel"
#	"gmp"
#	"libmpc"
#	"mpfr"
#)

#sudo pacman -Syu --noconfirm --needed "${PACKAGES[@]}"

# Setup the directory tree
PREFIX="/usr/local/cross"
TARGET=x86_64-elf
PATH="${PREFIX}/bin:${PATH}"

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

mkdir -p build/{binutils-gdb,gcc}

# Build binutils and gdb
cd build/binutils-gdb || exit 1
../../binutils-gdb/configure --target="${TARGET}" --prefix="${PREFIX}" --with-sysroot --disable-nls --disable-werror
make
sudo /bin/sh -c "make install"

# Build gcc
cd ../gcc || exit 1
../../gcc/configure --target="${TARGET}" --prefix="${PREFIX}" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
sudo /bin/sh -c "make install-gcc"
sudo /bin/sh -c "make install-target-libgcc"

# Add the cross compiler to PATH
export PATH="${PREFIX}/bin:${PATH}"

# Test the compiler
"${TARGET}"-gcc --version

# Done
echo "Done!"
echo "Don't forget to add ${PREFIX}/bin to your PATH variable in your .bashrc file!"
