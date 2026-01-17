FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    xorriso \
    qemu-system-x86 \
    wget \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install cross-compiler dependencies
RUN apt-get update && apt-get install -y \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    && rm -rf /var/lib/apt/lists/*

# Create build directory
WORKDIR /build

# Download and build cross-compiler (this takes a while)
RUN mkdir -p /opt/cross && \
    cd /tmp && \
    wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz && \
    wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz && \
    tar -xf binutils-2.40.tar.xz && \
    tar -xf gcc-13.2.0.tar.xz

# Build binutils
RUN cd /tmp && \
    mkdir build-binutils && \
    cd build-binutils && \
    ../binutils-2.40/configure --target=x86_64-elf --prefix=/opt/cross --with-sysroot --disable-nls --disable-werror && \
    make -j$(nproc) && \
    make install

# Add cross-compiler to PATH
ENV PATH="/opt/cross/bin:${PATH}"

# Build GCC
RUN cd /tmp && \
    mkdir build-gcc && \
    cd build-gcc && \
    ../gcc-13.2.0/configure --target=x86_64-elf --prefix=/opt/cross --disable-nls --enable-languages=c,c++ --without-headers && \
    make -j$(nproc) all-gcc && \
    make -j$(nproc) all-target-libgcc && \
    make install-gcc && \
    make install-target-libgcc

# Clean up build files
RUN rm -rf /tmp/binutils-* /tmp/gcc-* /tmp/build-*

# Download Limine bootloader
RUN cd /opt && \
    wget https://github.com/limine-bootloader/limine/releases/download/v4.20231210.0/limine-4.20231210.0.tar.xz && \
    tar -xf limine-4.20231210.0.tar.xz && \
    cd limine-4.20231210.0 && \
    make && \
    cp limine-cd.bin limine-cd-efi.bin limine.sys /opt/ && \
    cd .. && \
    rm -rf limine-4.20231210.0*

# Set working directory
WORKDIR /build

# Copy build script
COPY docker-build.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-build.sh

CMD ["/usr/local/bin/docker-build.sh"]