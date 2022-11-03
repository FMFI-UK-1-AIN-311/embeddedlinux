Kernel and userspace on RPi
===========================

Let's try to get the kernel and busybox working on a real RPi, similarly to what
we did before with qemu.

**Note: we will use the "softfloat" (no "hf") crosscompiler version from ubuntu,
because the hardfloat version produces "incompatible" binaries
[^ubuntu-arm-hf-rpi].**


## Preparation

We need two things to prepare when working with a real RPi a opposed to the
emulator.

### Formatting the SD card

We need to create to partitions on the sd card: a FAT "boot" partition and an
ext4[^rootfs-type] "rootfs" partition. The instructions will show how to do with
`fdisk`, but you could use other software to partition it (`parted` is a more
user friendly console program, GParted and KDE Partition Manager are gui
programs).

Check this [old guide on how to partition and format the SD
card](https://dai.fmph.uniba.sk/~siska/embeddedlinux/sdcard.php)

TODO move the guide here ?

After partitioning, mount the partitions. The following commands assume that the
partitions will be mounted at `/media/${USER}/boot` and `/media/${USER}/rootfs`.

### Connecting the serial console via a serial-to-USB converter

TODO how to connect;)

We will need a serial terminal emulator program. We will use `picocom`, install
it with `apt install picocom`.

## Building the kernel and busybox

The bootloader on RPi needs some special "firmware" files that it loads before
loading the kernel, so let's fetch them first

    mkdir -p firmware
    cd firmware
    wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
    wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat
    wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
    cd ..


Although the mainline kernel has some support for RPi 1 / Zero, we'll fetch the
RPi version of the kernel, so things are easier... The defconfig in these
sources is called `bcmrpi_defconfig`, the rest of building the kernel is the
same as before.

    wget https://github.com/raspberrypi/linux/archive/refs/heads/rpi-6.1.y.tar.gz
    tar xf rpi-6.1.y.tar.gz
    cd linux-rpi-6.1.y/
    export ARCH=arm
    export CROSS_COMPILE=arm-linux-gnueabi-
    make help | grep rpi
    make bcmrpi_defconfig
    make -j$(nproc) zImage dtbs


Lets copy over the kernel itself (it needs to be named `kernel.img` for RPi's
bootloader), the device trees (we really only need the correct RPi Zero W one,
but let's copy all of them) and also the firmware files


    cp -ar ~/skola/embeddedlinux/pi/firmware/*  /media/${USER}/boot/
    cp -ar arch/arm/boot/zImage  /media/${USER}/boot/kernel.img
    cp -ar arch/arm/boot/dts/*.dtb /media/${USER}/boot/

We also need the device tree overlays (those go into an `overlays`
subdirectory). Again, we really need only one of them, but let's copy all:

    mkdir /media/${USER}/boot/overlays/
    cp -arv arch/arm/boot/dts/overlays/*.dtb{,o} /media/${USER}/boot/overlays/

Next, set up the `config.txt` and `cmdline.txt` files. On RPis with bluetooth
(such as Zero W), the main serial port will be redirected to bluetooth, so we need to
"redirect" it back to the GPIO pins using the `miniuar-bt` overlay (TODO
explanation...)

    echo "dtoverlay=miniuart-bt" > /media/${USER}/boot/config.txt
    echo "8250.nr_uarts=1 earlyprintk console=ttyAMA0,115200 root=/dev/mmcblk0p2 rootwait init=/sbin/init" > /media/${USER}/boot/cmdline.txt


Next build busybox

    cd ..
    tar xf busybox-1.35.0.tar.bz2
    cd busybox-1.35.0/
    make defconfig
    make menuconfig # set path to self and static build
    make -j$(nproc)
    make install

    rsync -av _install/ /media/${USER}/rootfs/

Let's also create some important directories that various linux parts expect to
exist, to save us some problems later (`/dev/` is especially needed for
`/dev/console` to exist, thanks to the automount of the [`devtmpfs`
filesystem](https://www.google.com/search?q=devtmpfs)).

    mkdir /media/${USER}/rootfs/{dev,sys,proc}

## Booting the Pi

Unmount the partitions, eject the card and put it into the RPi

    sync
    umount /media/${USER}/boot/
    umount /media/${USER}/rootfs


Run the terminal emulator, check the device name on your computer. You will
also have to add your user to the `dialout` group (or run the command as root)
to have access to the device

    picocom -b 115200 /dev/ttyUSB0

RPi should boot the kernel and print it's logs to the serial console, including
the running `init` from busybox.

    [    4.693390] Run /sbin/init as init process
    can't run '/etc/init.d/rcS': No such file or directory

    Please press Enter to activate this console.

