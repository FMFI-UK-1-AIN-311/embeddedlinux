Lab 4
=====

We want to play a bit with the RPi foundation provided linux OS before we start
playing with our own. For that we want to do the following:

- Download Raspberry Pi OS and "flash" it to an sd card.
- Set up the system on the sd card for headless boot:
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

    wget https://downloads.raspberrypi.org/raspios_lite_armhf/images/raspios_lite_armhf-2022-09-26/2022-09-22-raspios-bullseye-armhf-lite.img.xz

Unpack it (while preserving the compressed file for future):

    unxz  <2022-09-22-raspios-bullseye-armhf-lite.img.xz  >2022-09-22-raspios-bullseye-armhf-lite.img

### Finding the sd card

To find out which device corresponds to your sd card, you can use `lsblk`. The
easiest way is to run it before and after you insert the sd card and check which
device(s) were added. Depending on how the card was formatted you can see a
single device without partitions, or a device with one or more partitions. Each
of them will appear as another (sub-) device.


    lsblk
    NAME          MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINTS
    ...
    mmcblk0       179:0    0  14,8G  0 disk  
    ├─mmcblk0p1   179:1    0   256M  0 part  
    └─mmcblk0p2   179:2    0   1,5G  0 part  

If the card was used with RPi before, it will most probably contain two
partitions: a smaller FAT partition named `boot` and a larger ext4 partition
named `rootfs` (you can see the filesystem type and label with `lsblk -f`).

Some linux distros automatically mount removable devices ;(. If that happens,
you must first umount them. `lsblk` will show the mountpoints if the partitions
were  mounted:

    NAME          MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINTS
    mmcblk0       179:0    0  14,8G  0 disk  
    ├─mmcblk0p1   179:1    0   256M  0 part  /media/yoyo/boot
    └─mmcblk0p2   179:2    0   1,5G  0 part  /media/yoyo/rootfs

Note that using the unmount / safely remove functionality through your GUi might
also actually disconnect the sd card, so it's safer to unmount it with the
`umount` command (you might have to do it under `root` depending on your
system):

    umount /media/yoyo/boot
    umount /media/yoyo/rootfs

### Flashing the image

You can use the `dd` command to actually copy the image to the card. You want to
use the device representing the whole sd card:

    dd if=2022-09-22-raspios-bullseye-armhf-lite.img of=/dev/mmcblk0 bs=1M


Set up headless booting with ethernet over USB
----------------------------------------------

Mount the `boot` partition after writing the sd card image. You can do this
through your GUI or with the following commands (you will have to do it as root
and then only root will be able to access the mounted filesystem):

    mkdir /mnt/boot
    mount /dev/mmcblk0p1 /mnt/boot

The following text will assume that the `boot` partition is mounted on
`/media/yoyo/boot`.

### Set up ssh to start automatically

Just create an empty file named ssh:

    touch /media/yoyo/boot/ssh

### Set up a user and password

Create a file name `userconf.txt` with the contents `username:hashed_password`.
Use the following command to get the hashed password (see the
[Setting up a Headless Raspberry Pi](https://www.raspberrypi.com/documentation/computers/configuration.html#configuring-a-user)
section of the RPi docs):

    openssl passwd -6

### Set up usb networking

Add `dtoverlay=dwc2` at the end of `config.txt`

Add `modules-load=dwc2,g_ether` to `cmdline.txt`, just after the `rootwait` option.


Booting the RPi
---------------

Unmount the sd card and insert it into RPi. Connect it to your computer. A USB
network adapter should appear after some time. Configure it to allow "automatic"
/ link local IPv4 configuration. You should then be able to connect to RPi with
(use the user you configured in `userconf.txt`):

    ssh user@raspberrypi.local

Note: you can set up a ssh key and copy the public key over to RPi to make it
easier to connect without typing a password. TODO link


Cross-compile a program and run it
----------------------------------

On ubuntu systems, you can easily install an arm cross-compiler with

   apt install gcc-arm-linux-gnueabihf

Additionally you can install `g++-arm-linux-gnueabihf` if you also want the C++
cross-compiler.

Create a simple C program `hello.c`:

```c
#include <stdio.h>

int main() {
	printf("hello\n");
	return 0;
}
```

Compile it with the cross-compiler:

    arm-linux-gnueabihf-gcc -Wall -o hello hello.c

    file hello
    hello: ELF 32-bit LSB pie executable, ARM, EABI5 version 1 (SYSV), dynamically linked,
    interpreter /lib/ld-linux-armhf.so.3, BuildID[sha1]=f0f24a02ca790cd36d2e2ed3ecf4fac5550864c5,
    for GNU/Linux 3.2.0, not stripped

Copy it over to the RPi:

    scp hello user@raspberrypi.local:

Log in to RPi again using SSH and run the program.
