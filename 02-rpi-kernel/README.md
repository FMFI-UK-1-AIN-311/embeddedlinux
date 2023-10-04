Kernel on RPi
=============

Let's try to compile our own kernel for RPi.

**Note: we will use the "softfloat" (no "hf") crosscompiler version from ubuntu,
because the hardfloat version produces "incompatible" binaries
[^ubuntu-arm-hf-rpi].**


## Preparation

We need a couple of things to prepare when working with a real RPi:
  - a SD card with a "boot" partition,
  - low level firmware for RPi that allows it to boot the kernel,
  - and finally compile th actual kernel (along with device tree information,
	which is a description of the actual hardware for the kernel.)

### Formatting the SD card

We need to create to partitions on the sd card: a FAT "boot" partition and an
ext4[^rootfs-type] "rootfs" partition. The instructions will show how to do with
`fdisk`, but you could use other software to partition it (`parted` is a more
user friendly console program, GParted and KDE Partition Manager are gui
programs).

Check this [old guide on how to partition and format the SD
card](https://dai.fmph.uniba.sk/~siska/embeddedlinux/sdcard.php)

TODO move the guide here ?

Note: it uses `boot` as the name of the boot partition, while the new version
of RPi OS use `bootfs`, so we will use the new name throughout this guide.

After partitioning, mount the partitions. The following commands assume that the
partitions will be mounted at `/media/${USER}/bootfs` and `/media/${USER}/rootfs`.

### Connecting the serial console via a serial-to-USB converter

TODO how to connect;)

We will need a serial terminal emulator program. We will use `picocom`, see
[Computer setup](../00-intro/#computer-setup).

## Preparing firmware

The bootloader on RPi needs some special "firmware" files that it loads before
loading the kernel, so let's fetch them first

    mkdir -p firmware
    cd firmware
    wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
    wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat
    wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
    cd ..


## Building the kernel

Linux kernel needs openssl headers for some crypto stuff (TODO should be
possible to turn off?), so we need to install the headers

    apt install libssl-dev

Although the mainline kernel has some support for RPi 1 / Zero, we'll fetch the
RPi version of the kernel, so things are easier...

Download the sources from github (we don't want to clone the whole git repo
with history right now ;)

    wget https://github.com/raspberrypi/linux/archive/refs/heads/rpi-6.1.y.tar.gz

Unpack it

    tar xf rpi-6.1.y.tar.gz

and change into its directory:

    cd linux-rpi-6.1.y/

We need to set up a couple of environment variables to tell the kernel build
system that we are cross-compiling:

    export ARCH=arm
    export CROSS_COMPILE=arm-linux-gnueabi-

With the proper architecture set, we can check which boards the kernel has
"prepared" configs (well, we'll just grep for the rpi one here, you can look
at the full `make help` output if you want ;)

    make help | grep rpi

The defconfig in these sources for RPi is called `bcmrpi_defconfig`, so let's
apply that:

    make bcmrpi_defconfig

Now that the build system is properly configured, we can build the actual
kernel (`zImage` is a gzipped version, see `make help` for other options.)

    make -j$(nproc) zImage dtbs

Note: running `make -j$(nproc)` without further argument would run the
"default" build, which also includes modules, which takes a lot of time and we
don't need them now, so we just build the kernel image itself and "hadrware
description" files - device trees): 

## Copy files to the boot partition

Lets copy over the kernel itself (it needs to be named `kernel.img` for RPi's
bootloader), the device trees (we really only need the correct RPi Zero W one,
but let's copy all of them) and also the firmware files


    cp -ar ../firmware/*  /media/${USER}/bootfs/
    cp -ar arch/arm/boot/zImage  /media/${USER}/bootfs/kernel.img
    cp -ar arch/arm/boot/dts/*.dtb /media/${USER}/bootfs/

We also need the device tree overlays (those go into an `overlays`
subdirectory). Again, we really need only one of them, but let's copy all:

    mkdir /media/${USER}/bootfs/overlays/
    cp -arv arch/arm/boot/dts/overlays/*.dtb{,o} /media/${USER}/bootfs/overlays/

Next, set up the `config.txt` and `cmdline.txt` files. On RPis with bluetooth
(such as Zero W), the main serial port will be redirected to bluetooth, so we need to
"redirect" it back to the GPIO pins using the `miniuar-bt` overlay (TODO
explanation...)

    echo "dtoverlay=miniuart-bt" > /media/${USER}/bootfs/config.txt
    echo "8250.nr_uarts=1 earlyprintk console=ttyAMA0,115200 root=/dev/mmcblk0p2 rootwait init=/sbin/init" > /media/${USER}/bootfs/cmdline.txt


TODO explain the commandline ;)

## Root filesystem partition

We don't really need a rootfs right now ;). IF you used the SD card from
previous experiments with RPi OS, it would contain a rootfs with that OS. IF
you don't delete it, your kernel should actually boot into it ;)

You can erase it / create an empty rootfs partition with (make sure the
partition isn't mounted)

    mkfs.ext4 -L rootfs /dev/${your_device_for_the_rootfs_partition?}

## Booting the Pi

Unmount the partitions, eject the card and put it into the RPi

    sync
    umount /media/${USER}/boot/
    umount /media/${USER}/rootfs


Run the terminal emulator, check the device name on your computer:

    picocom -b 115200 /dev/ttyUSB0

RPi should boot the kernel and print its logs to the serial console.  With no
rootfs filesystem / partition, it should complain about not being able to mount
the root filesystem. With an empty rootfs partition it will complain about not
having an `init` process.


[^ubuntu-arm-hf-rpi]: While RPi zero (or RPi 1) has hardware floating point
  support, the hardfloat arm toolchain present in ubuntu is by default
  configured for a newer arm version (TODO: could it just confingured with
  correct options to produce correct code? I didn't manage to do that.) The
  "softfloat" toolchain generates code for the older cpu and in our test we
  won't really need any floating point math (much.)  If you want a hardfloat
  support, you can
  [build your own cross compiler](https://dai.fmph.uniba.sk/~yoyo/embeddedlinux/toolchain.php)
  or (google and) download some other pre-compiled toolchain with propper support.
