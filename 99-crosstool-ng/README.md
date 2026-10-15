Compile a cross-compilation toolchain with crosstool-ng
=======================================================

Install dependencies
--------------------

For ubuntu:

    sudo apt install gperf bison flex texinfo help2man make libncurses5-dev \
        python3-dev autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip \
        patch libstdc++6 rsync git meson ninja-build

For other systems you can check the Dockerfiles in
[https://github.com/crosstool-ng/crosstool-ng/tree/master/testing/docker](https://github.com/crosstool-ng/crosstool-ng/tree/master/testing/docker).

Get crosstool-ng
----------------

### Option 1: download a specific release

    wget https://github.com/crosstool-ng/crosstool-ng/releases/download/crosstool-ng-1.29.0/crosstool-ng-1.29.0.tar.bz2
    tar xf crosstool-ng-1.29.0.tar.bz2
    cd crosstool-ng-1.29.0/

### Option 2: check out the latest version from github

In this case we need to also run `bootstrap.sh`

    git clone https://github.com/crosstool-ng/crosstool-ng
    cd crosstool-ng/
    ./bootstrap


Configure / "install" crosstool-ng
----------------------------------

We are going to configure / install it locally directly in the unpacked folder:

    ./configure  --enable-local
    make


Configure our toolchain
-----------------------

Configure the defaults for RPI zero targets:

    ./ct-ng armv6-unknown-linux-gnueabihf

After that we can run `menuconfig` to fine tune some things

    ./ct-ng menuconfig

In there: we might want to change the directories, where the source downloads will be
placed:

    Paths and misc options  --->
      Local tarballs directory
      Prefix directory

I like to set the tarballs directory to `${HOME}/skola/embeddedlinux/src/ct` and the
prefix directory (where the compiled toolchain will be placed) to
`${CT_PREFIX:-${HOME}/skola/embeddedlinux/cross}/${CT_HOST:+HOST-${CT_HOST}/}${CT_TARGET}`
(I just inserted `skola/embeddedlinux` into the default `${HOME}/cross/....`).

### Optional: change the kernel headers version

The toolchain is compiled against a specific version of kernel headers. It will
work on any newer kernel, but will not work on an older kernel. Making it too
old will however make it impossible for the libc to use newer, more efficient
kernel features.

Currently the default for armv6-unknown-linux-gnueabihf is 5.4, which should be
fine, but you can change it in (the linux kernel version we will be compiling
for RPi will be probably 6.18):

    Operating System  --->
      Version of linux

### Optional: change the glibc version

The toolchain is compiled agains a specific version of the C library (in our
case GNU libc also named glibc). Binaries compiled with the tool chain will work
on any system that has that or newer version of libc, but will not work on
systems with older glibc.

The default crosstool-ng uses the most recent glibc version. That will be fine
for our system that we compile from scratch, but if you want to compile programs
that will run for example on the RPi provided OS, you will need to lower the
version to match that (google
[glibc version in latest debian releases](https://www.google.com/search?q=glibc+version+in+latest+debian+releases)
).

    C-library  --->
      Version of glibc (2.44)  --->


Build the toolchain
-------------------

    time ./ct-ng build

On an old 6-core Intel laptop:

    real    35m54.579s
    user    226m9.764s
    sys     19m53.080s

On a recent 20 core Intel laptop:

    real    17m52,921s
    user    143m40,099s
    sys     20m48,916s

Used space:

    du -sxhc crosstool-ng-1.29.0 ~/skola/embeddedlinux/{src/ct,cross/armv6-rpi-linux-gnueabihf}

    14G     crosstool-ng-1.29.0
    351M    /home/yoyo/skola/embeddedlinux/src/ct
    623M    /home/yoyo/skola/embeddedlinux/cross/armv6-rpi-linux-gnueabihf
    15G     total


The first folder is the build directory, the seconds is the downloads folder,
the third is the final produced toolchain.

Note about naming: the sample configuration we used was named
`armv6-unknown-linux-gnueabihf` but inside it defines the vendor as `rpi` so the
built toolchain is named `armv6-rpi-linux-gnueabihf`.

Testing it
----------

I told ct-ng to install it under
`${HOME}/skola/embeddedlinux/cross/armv6-rpi-linux-gnueabihf/`, so let's make it
available in our `$PATH`:

    export PATH="$PATH:${HOME}/skola/embeddedlinux/cross/armv6-rpi-linux-gnueabihf/bin/"

Let's create a small test file test.c

```c++
#include <stdio.h>

int main(int argc, char* argv[])
{
    printf("Hi there!\n");
    return 0;
}
```

and compile it

    armv6-rpi-linux-gnueabihf-gcc -static -o test test.c

Let's see what we got

    file test
    test: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), statically linked, BuildID[sha1]=5b2f77fafa196372db70b0e52364f25f69aac789, for GNU/Linux 6.18.35, with debug_info, not stripped

and try to run it

    ./test
    -bash: ./test: cannot execute binary file: Exec format error

We could try to install an arm emulator (on ubuntu):

    sudo apt install qemu-user

    qemu-arm ./test
    Hi there!

The qemu-arm package actually set up the system to actually always run
the emulator when we try to execute arm executables:

    ./test
    Hi there!

Note: I compiled it statically above to avoid problems with the dynamic loader
when running the binary under emulator.
