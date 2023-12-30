
# Install necessary packages, but dont reinstall them if they are already installed
sudo pacman -Syu --noconfirm --needed \
	base-devel \
	gmp \
	libmpc \
	mpfr

# Setup the directory tree
BASE_DIR=$(realpath ../../Binux)

# Confirm that the root directory is correct if it seems incorrect
if ! [[ "$(basename ${BASE_DIR})" == "Binux" ]]; then

	echo "The root directory is $BASE_DIR. Is this correct? [Y/n]"
	read -r CONFIRM

	if [[ "${CONFIRM,,}" == "n"* ]]; then
		echo "Please edit the BASE_DIR variable in this script."
		exit 1
	fi

else
	echo "The root directory is $BASE_DIR."
fi

exit 2

PREFIX="$BASE_DIR/opt/cross"
TARGET=aarch64-elf
PATH="$PREFIX/bin:$PATH"

mkdir -p "$BASE_DIR"/{src,opt/cross/bin}

# Download the source code
cd "$BASE_DIR"/src
wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz

# Extract the source code
tar -xf binutils-2.41.tar.gz
tar -xf gcc-13.2.0.tar.gz

mkdir build-binutils build-gcc

# Build binutils
cd build-binutils
../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Build gcc
cd ../build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

# Add the cross compiler to PATH
export PATH="$BASE_DIR/opt/cross/bin:$PATH"

# Test the compiler
cd "$BASE_DIR"
$TARGET-gcc --version

#echo 'int main(){}' > test.c
#$TARGET-gcc -o test test.c
#file test
#rm -f test.c test