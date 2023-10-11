Busybox on RPi
==============

Now that we have a running kernel and (for now empty) root filesystem, we can
compile some "userspace" programs that would allow us to actually do someting.

Init
----

When the kernel boots and mounts the root filesystem, it then starts the first
userspace process. This will have the process ID of 1, it must not exit/finish
and is responsible for starting (end "reaping", but more on that later) all
other processes..

Lets compile (statically) the "hello world" program we tried in the first labs,
put it on the rootfs partiion (named say `hello`) and modify `commandline.txt`
(in bootfs partition) to include `init=/hello`.

When you boot your RPi, the kernel will mount the root filesystem, run the
`hello` as the init process (you should see the "hello" output) and then panic
again, because the init process exits.

Busybox
-------

Busybox is a minimal implementation of common linux / posix utilities needed
to run a very minimal linux system.

You can download it from http://busybox.net/

    wget https://busybox.net/downloads/busybox-1.36.1.tar.bz2

Unpack it

    tar xf busybox-1.36.1.tar.bz2

and chage to the unpacked directory

    cd busybox-1.36.1/

Busybox uses the same build system as linux kernel, though it doesn't really
have different defconfigs. Set the variables required for cross-compilation
as we did for kernel

    export ARCH=arm
    export CROSS_COMPILE=arm-linux-gnueabi-

and configure it with defaults

    make defconfig

After that, run the manual configuration

    make menuconfig

and change the following options as shown:

    Settings  --->
      (/bin/busybox) Path to busybox executable
      [*] Build static binary (no shared libs)

The first option (`Path to busybox executable`) is used by busybox binaries when
they need to re-execute themselves, but the default value (`/proc/self/exe`)
won't work until we mount the `proc` filesystem, so we change that to an
explicit path to the binary (we know where we are going to install/copy it.)

The second option is needed, because we are not yet going to compile and install
the C library, so we need all binaries to be statically compiled.

Now we can build it

    make -j$(nproc)

Afterwards call

    make install

which will create a direcotry named `_install` which has all the files prepared
to be copied to our target (i.e. the rootfs.) It will contain a single binary
`/bin/busybox` and a lot of symbolic links to it with different names. This is
because busybox builds only one binary that does different things based on what
name it was invoked as. If it is invoked through the `ls` symlink, it will
execute the `ls` "function".

Let's copy it over to our rootfs. This step expects that you [re-formatted the
rootfs partition previously](../02-rpi-kernel#root-filesystem-partition).
Depending on how you mounted the ext4 filesystem, you might need to run the following
command as root (`sudo rsync...`)

    rsync -av _install/ /media/${USER}/rootfs/

Let's also create some important directories that various linux parts expect to
exist, to save us some problems later (`/dev/` is especially needed for
`/dev/console` to exist, thanks to the automount of the [`devtmpfs`
filesystem](https://www.google.com/search?q=devtmpfs)).

    mkdir /media/${USER}/rootfs/{dev,sys,proc}

## Booting the Pi

*Note: if you changed the `init` parameter in `commandline.txt` to test your
"init", change it back to `/sbin/init` (or just remove it completely.)*

Unmount the partitions, eject the card and put it into the RPi.

Run the terminal emulator, check the device name on your computer. You will
also have to add your user to the `dialout` group (or run the command as root)
to have access to the device

    picocom -b 115200 /dev/ttyUSB0

RPi should boot the kernel and print it's logs to the serial console, including
the running `init` from busybox.

    [    4.693390] Run /sbin/init as init process
    can't run '/etc/init.d/rcS': No such file or directory

    Please press Enter to activate this console.

When you press Enter, you should get a shell prompt and you can enjoy your own
linux system ;).

TODO for the quick ones: [more userspace (old labs)](../lab08#more-userspace)
