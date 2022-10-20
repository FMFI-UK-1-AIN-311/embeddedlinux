Linux in a virtual machine
==========================

[Old guide](https://dai.fmph.uniba.sk/~siska/embeddedlinux/qemu.php)

We will use the arm crosscompiler from ubuntu we installed earlier (or you can use
one you built with crosstool-ng, or download one).

Install QEMU 

- from system

    apt install qemu-system-arm

- [compile from source](https://www.qemu.org/download/)

    wget https://download.qemu.org/qemu-7.1.0.tar.xz
    tar xvJf qemu-7.1.0.tar.xz
    cd qemu-7.1.0

    ./configure --target-list=arm-softmmu,arm-linux-user --prefix=$(HOME)/embeddedlinux/usr
    make -j5
    make install

Download linux (kernel) sources

    wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.0.2.tar.xz

Unpack

    tar xf src/linux-6.0.2.tar.xz

We should have the sources unpacked

    cd linux-6.0.2/
    ls

```
arch   certs    CREDITS  Documentation  fs       init      ipc     Kconfig  lib       MAINTAINERS  mm   README   scripts   sound  usr
kblock  COPYING  crypto   drivers        include  io_uring  Kbuild  kernel   LICENSES  Makefile     net  samples  security  tools  virt
```

We need to set up a couple of environment variables to tell the kernel build system that
we are cross-compiling:

    export ARCH=arm
    export CROSS_COMPILE=arm-none-linux-gnueabi-
 
We will be using a configuration for a virtual cpu that comes with kernel in the form of a
"defconfing" (default configuration for a specific machine / type, you can use `make help` to
get a list of available ones).

    make versatile_defconfig

Build the kernel image (note: running `make -j$(nproc)` without further argument would run the
"default" build, which also includes modules, which takes a lot of time and we don't need them
now, so we just build the kernel image itself and "hadrware description" files - device trees): 

    make -j$(nproc) zImage dtbs

Run our new kernel in an emulator

    qemu-system-arm  \
        -M versatilepb \
        -nographic \
        -kernel arch/arm/boot/zImage \
        -dtb arch/arm/boot/dts/versatile-pb.dtb \
        -append console=ttyAMA0

```
...
Booting Linux on physical CPU 0x0
Linux version 6.0.2 (yoyo@yx) (arm-linux-gnueabihf-gcc (Ubuntu 11.2.0-17ubuntu1) 11.2.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #1 Thu Oct 20 10:34:04 CEST 2022
CPU: ARM926EJ-S [41069265] revision 5 (ARMv5TEJ), cr=00093177
...
VFS: Cannot open root device "(null)" or unknown-block(0,0): error -6
Please append a correct "root=" boot option; here are the available partitions:
...
---[ end Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0) ]---
```

This is expected, we didn't provide any filesystem and linux needs to mount a root filesystem
and execute the first / main (pid 1) "init" process from init.

To quit the emulator press `Ctrl-A` and then `X`.
