Raspberry Pi OS with network over usb
=====================================

We want to play a bit with the RPi foundation provided linux OS before we start
playing with our own. For that we want to do the following:

- Download Raspberry Pi OS and "flash" it to a SD card.
- Set up the system on the SD card for headless boot:
  - configure it to behave like a USB network card, so we can connect to it
    directly over the USB cable
  - set up ssh to start on boot
  - set up a user and password
- Boot it and play around a bit...
- Install / donwload / compile a cross compiler for rpi
- Crosscompile a simple program and run it on rpi


Donwloading Raspberry Pi OS
---------------------------

Go to https://www.raspberrypi.com/software/operating-systems/ and download
Raspberry Pi OS Lite. From the commandline you can do (note: the link was valid
as of 12.10.2022, check the current version on the page):

    wget https://downloads.raspberrypi.org/raspios_lite_armhf/images/raspios_lite_armhf-2023-05-03/2023-05-03-raspios-bullseye-armhf-lite.img.xz

Unpack it (while preserving the compressed file for future):

    unxz -k 2023-05-03-raspios-bullseye-armhf-lite.img.xz

the `-k` option will also keep the original compressed `.xz` file, but if you
don't need, you can just ommit it and `unxz` will remove the compressed file
after decompressing it.


### Finding the SD card

To find out which device corresponds to your SD card, you can use `lsblk`. The
easiest way is to run it before and after you insert the SD card and check which
device(s) were added. Depending on how the card was formatted you can see a
single device without partitions, or a device with one or more partitions. Each
of them will appear as another (sub-) device.


    lsblk
    NAME          MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINTS
    ...
    mmcblk0       179:0    0  29,7G  0 disk  
    ├─mmcblk0p1   179:1    0   256M  0 part  
    └─mmcblk0p2   179:2    0   1,6G  0 part  

If the card was used with RPi before, it will most probably contain two
partitions: a smaller FAT partition named `boot` and a larger ext4 partition
named `rootfs` (you can see the filesystem type and label with `lsblk -f`).

If you use a USB card reader, the device will most probably be `sda` (or `sdb`,
`sdc`..) instead of `mmcblk0`, however be careful since other (SATA or USB)
drives follow the same naming scheme.

Some linux distros automatically mount removable devices ;(. If that happens,
you must first umount them. `lsblk` will show the mountpoints if the partitions
were  mounted:

    NAME          MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINTS
    mmcblk0       179:0    0  29,7G  0 disk  
    ├─mmcblk0p1   179:1    0   256M  0 part  /media/yoyo/bootfs
    └─mmcblk0p2   179:2    0   1,6G  0 part  /media/yoyo/rootfs

Note that using the unmount / safely remove functionality through your GUI might
also actually disconnect the SD card, so it's safer to unmount it with the
`umount` command (you might have to do it under `root` depending on your
system):

    umount /media/yoyo/bootfs
    umount /media/yoyo/rootfs

### Flashing the image

You can use the `dd` command to actually copy the image to the card. You want to
use the device representing the whole SD card:

    dd if=2023-05-03-raspios-bullseye-armhf-lite.img of=/dev/mmcblk0 bs=1M status=progress

Set up headless booting with ethernet over USB
----------------------------------------------

Mount the `bootfs` partition after writing the SD card image. You can do this
through your GUI (removing and re-inserting the SD card might be the fastest way
to trigger the GUI action / automount) or with the following commands (you will
have to do it as root and then only root will be able to access the mounted
filesystem):

    mkdir /mnt/boot
    mount /dev/mmcblk0p1 /mnt/boot

The following text will assume that the `boot` partition is mounted on
`/media/yoyo/bootfs`.

### Set up ssh to start automatically

Just create an empty file named ssh:

    touch /media/yoyo/bootfs/ssh

### Set up a user and password

Create a file named `userconf.txt` with the contents `username:hashed_password`.
Use the following command to get the hashed password (see the
[Setting up a Headless Raspberry Pi](https://www.raspberrypi.com/documentation/computers/configuration.html#configuring-a-user)
section of the RPi docs):

    openssl passwd -6

You can set it up directly with the following command:

    echo "pi:$(openssl passwd -6)" > /media/yoyo/bootfs/userconf.txt

### Set up usb networking

Add `dtoverlay=dwc2` at the end of `config.txt`:

    echo "dtoverlay=dwc2" >>/media/yoyo/bootfs/config.txt

Add `modules-load=dwc2,g_ether` to `cmdline.txt`, just after the `rootwait` option:

    sed -i -e "s/rootwait/rootwait modules-load=dwc2,g_ether/" /media/yoyo/bootfs/cmdline.txt


Booting the RPi
---------------

Unmount the SD card and insert it into RPi. Connect it to your computer. A USB
network adapter should appear after some time (the first boot might take longer,
because the OS will resize the small filesystem to fill the whole SD card.)

Configure the network on this new adapter to allow "automatic" / link local IPv4
configuration. You should then be able to connect to RPi as the user you
configured in `userconf.txt`:

    ssh pi@raspberrypi.local

Note: you can set up a ssh key and copy the public key over to RPi to make it
easier to connect without typing a password. TODO link


Cross-compile a program and run it
----------------------------------

On ubuntu systems, you can easily install an arm cross-compiler with

    apt install gcc-arm-linux-gnueabi

Additionally you can install `g++-arm-linux-gnueabi` if you also want the C++
cross-compiler.

*Note: it might be better to open two terminal windows / tabs, one for the ssh
connection to RPi, one for the "local" steps, instead of connecting and
disconnecting from RPi every time you want to do something locally ;-)*

Create a simple C program `hello.c`:

```c
#include <stdio.h>

int main() {
	printf("hello\n");
	return 0;
}
```

Compile it with the cross-compiler:

    arm-linux-gnueabi-gcc -Wall -o hello hello.c

    file hello
    hello: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked,
    interpreter /lib/ld-linux.so.3, BuildID[sha1]=78f8b43615a9888352fe3ffdbe80b5225d8ee17a,
    for GNU/Linux 3.2.0, not stripped


Copy it over to the RPi:

    scp hello pi@raspberrypi.local:

Log in to RPi again using SSH and run the program.

And that's it.

...

Or not...

If you used a (relatively) new linux distro, you might have ended with:

    pi@raspberrypi:~ $ ./hello
    ./hello: /lib/arm-linux-gnueabihf/libc.so.6: version `GLIBC_2.34' not found (required by ./hello)

The problem is, that the cross-compiler you used (arm-linux-gnueabi-gcc) was
compiled with a newer libc library (2.34) than what the Raspberry Pi OS uses.

We can check what we have on the RPi:

    pi@raspberrypi:~ $ dpkg -l libc6
    Desired=Unknown/Install/Remove/Purge/Hold
    | Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
    |/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
    ||/ Name           Version                   Architecture Description
    +++-==============-=========================-============-=================================
    ii  libc6:armhf    2.31-13+rpt2+rpi1+deb11u5 armhf        GNU C Library: Shared libraries

Once we compile our own C library for the device, that won't be a problem (we
will compile a new one, yes?) For now we can get rid of the dependency on the C
library from the RPi OS by building a static library (that will have everything
compiled into it):

    arm-linux-gnueabi-gcc -Wall -static -o helloStatic hello.c

Compile, copy over to RPi and it should now run

    pi@raspberrypi:~ $ ./helloStatic 
    hello

although at the cost of a significant increase in size

    pi@raspberrypi:~ $ ls -l hello*
    -rwxr-xr-x 1 pi pi   7708 May  3 01:44 hello
    -rwxr-xr-x 1 pi pi 580420 May  3 01:43 helloStatic

