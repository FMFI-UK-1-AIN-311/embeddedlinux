Some userspace
==============

**In last labs I made a mistake... The `versatile-pb` deice / architecture doesn't
have hardware floating point oprations support, but we installed and used a `hf`
version crosscompiler... We either have to use a non-hf compiler (`apt install gcc-arm-linux-gnueabi`)
or use the vexpress machine, which has hardfloat support (both when configuring kernel and
when building our application, see below).**

**If you already have everything working from last week, the best way is to just go the vexpress
route, just start again by reconfiguring and recompiling the kernel**

Short recap
-----------


### Common requirements

Install common deps, TODO there are other, I just remember these ;)

    sudo apt update
    sudo apt install gcc flex bison
    sudo apt install qemu-system-arm

Download and unpack kernel source

    wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.0.2.tar.xz
    tar xf src/linux-6.0.2.tar.xz

Change to the linux directory

    cd linux-6.0.2/   

Prepare environment for cross-compilation

    export ARCH=arm
    export CROSS_COMPILE=arm-none-linux-gnueabi-

### Versatile-pb (soft-float)

Install non-hf crosscompiler

    sudo apt install gcc-arm-linux-gnueabi

Prepare environment for cross-compilation

    export ARCH=arm
    export CROSS_COMPILE=arm-none-linux-gnueabi-

Configure kernel

    make versatile_defconfig

Build

    make -j$(nproc) zImage dtbs

Run virtual machine

    qemu-system-arm  \
        -M versatilepb \
        -nographic \
        -kernel arch/arm/boot/zImage \
        -dtb arch/arm/boot/dts/versatile-pb.dtb \
        -append console=ttyAMA0


### Vexpress (hard-float)

Install non-hf crosscompiler

    sudo apt install gcc-arm-linux-gnueabihf

Prepare environment for cross-compilation

    export ARCH=arm
    export CROSS_COMPILE=arm-none-linux-gnueabihf-

Configure kernel

    make vexpress_defconfig

Build

    make -j$(nproc) zImage dtbs

Run virtual machine

    qemu-system-arm  \
        -M vexpress-a15 \
        -nographic \
        -kernel arch/arm/boot/zImage \
        -dtb arch/arm/boot/dts/vexpress-v2p-ca15-tc1.dtb \
        -append console=ttyAMA0


The kernel now "panics" because it can't mout the root filesystem...


A built-in rootfs (initial ram disk)
------------------------------------

Create a direcotry, that will be our "initrd":

    mkdir initrd

Run 

    make menuconfig

and go to

    General setup --->

and configure the following (the first should be set)

        [*] Initial RAM filesystem and RAM disk (initramfs/initrd) support
        (initrd) Initramfs source file(s)

Now rebuild the kernel (no need to rebuild the device trees)

    make -j$(nproc) zImage

and run `qemu` again. This time it should mount the initrd and fail because it
can't find the init.


