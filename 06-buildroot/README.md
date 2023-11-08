Buildroot
=========

TODO [old page with info for a virtual machine](https://dai.fmph.uniba.sk/~siska/embeddedlinux/buildroot.php)

[Buildroot](https://buildroot.org/) is a tool, that automates the whole creation
of an embedded linux image: crosscompiler toolchain creation, kernel
configuration and build and packages building.

Buildroot has a database of many packages that can be built and included in the
image. All you have to do is configure it (using the kernel based KConfig) and
run it:

- clone / get+unpack buildroot
- run `make menuconfig`
  - select the machine / toolchain / libc / kernel etc...
  - select init system / dev management / busybox features etc...
  - select additional packages
- create an overlay with your config / additional files
- run `make`


## Basic configuration

    git clone git://git.buildroot.net/buildroot


    make list-defconfigs | grep raspberry

We'll select `raspberrypi0w_defconfig`:

    make raspberrypi0w_defconfig


By default `buildroot` will compile it's own toolchain, but that would take a log of time.
Let's change the toolchain to use a pre-compiled toolchain from the web. Run

    make menuconfig

and change

    Toolchain  --->
      Toolchain type  --->
        External toolchain

*Alternatively you could select  Custom toolchain in Toolchain and set prefix to
the ubuntu `arm-linux-gnueabi-` toolchain, but you will need to explicitly set
information about the toolchain like the version of gcc/libc/linux headers which
this toolchain uses...*


## Kernel and busybox configuration

Buildroot will use a pre-defined kernel and busybox config. You can `make
linux-menuconfig` or `make busybox-menuconfig` if you want to change anything in
their configuration.

## Package selection

Let's do the same thing as we did ourselves: busybox, mpg123, alsa-libs,
alsa-utils...

In `Target packages`, you can select individual packages that you want in your
image:

    Target packages  --->
      Audio and video applications  --->
        [*] alsa-utils  --->
          [*]   alsamixer
        [*] mpg123

Note that selecting any of the alsa utils automatically selected `alsa-lib`
under `Libraries` > `Audio/Sound` and `mpg123` will also automatically see it
when it is configured.

Also selecting `alsmixer` automatically selected the `ncurses` library, which we
didn't want to compile on our own and configured `alsa-utils` to skip this
tool.

## Add additional files to the image

    System configuration  --->
      Root filesystem overlay directories

This is a list of directories that will be copied over to the resulting rootfs
as is (create a direcotry with your mp3 file here and put the path to the
direcotry here ;)


## Build it

    make

Note: buildroot desn't build packages in parallel (i.e. as with `-j1`), however it
will build each package with a parallel make according to the `Build options` >
`Number of jobs to run simultaneously` setting (default is 0 which means to
autodect, so you really don't need to change anything).


The output you are looking for will be in output/images.
