# Intro

We will be working with Raspberry Pi Zero W.

## Running and compiling programs on linux

When a program runs, it talks to the kernel using a binary interface (ABI)
that's defined by the kernel headers. Normally a program doesn't use that
directly, instead it uses the "standard" C functions / interfaces and the C
library translates those to the actual kernel calls.

```mermaid
graph TD;
    kernel["Linux (kernel)"] --> libc[C library]
    headers[Kernel headers] -.-> libc
    libc --> app[Application]
```

A working "compiler" thus needs to be a bit more complicated then a program to
translate C syntax to assembly, because it needs to be tightly coupled with an
appropriate C library, which is in turn built against specific kernel headers.
Because of this, we usually talk about the `compilation toolchain` and not just
the compiler (this also usually includes additional tools that are used to
manipulate libraries etc.)

## Cross-compilation

The Raspberries use an arm processor which uses a different architecture and
instruction set than the "normal" Intel / AMD x86 processors. The compiler
normally produces binaries (machine code) for the architecture on which it runs
(i.e. x86 architecture for a "normal" desktop linux.) To compile programs for
another architecture (arm for Raspberry Pi), we need a different compiler, that
runs on x86 but produces binaries for arm.

Because the compiler needs to produce binaries that will run against a specific
kernel ABI (defined by specific kernel headers) and a specific C library, so for
a specific cross compiler there are minimal kernel and libc versions with which
the generated programs will run.

## Computer setup

We'll need a linux installation with access to a SD card reader and USB to serial adapter.
Native installation or a live USB is ok, virtual machine (VirtualBox, VMWare...)
works with a usb SD card reader (by attaching the usb devices to the guest OS.)

We will need to install some software that is not present on some distros by
default. The examples will assume ubuntu (or derivatives) and provide an
`apt-get install ...` command, for other distros you need to install equivalent
packages in appropriate way for your distro.

The USB to serial adapter creates a device (usually `/dev/ttyUSB0`) that is by
default accessible to members of the `dialout` group (you can check the
permissions with `ls -l /dev/ttyUSB0`), but by default the users in ubuntu are
not part of that group. To add your user to that group run the command

    sudo usermod -a -G dialout $USER

and then log out and log in again (note that the `-a` flag is **very**
important, do not forget it ;-). You can check which groups your user is member
of with the `id` command.

To communicate over the serial port we will use `picocom` a very simple terminal
emulator. Install it with

    sudo apt install picocom

and then try it (with the USB to serial adapter plugged into your computer) with

    picocom -b 115200 /dev/ttyUSB0
