Kernel and userspace on RPi
===========================


- Get busybox running on the RPi ([from last week](../lab07/))
- Automate the build
- Some more userspace

## Automate the build

We could just put commands in a script (set up to exit on first failure!), but
if we change some things, it will always run from the beginning regardless of
whether it needs to.

`make` is a program that uses uses rules to build files from other files and uses
timestamps to detect what really needs to be done and what can be avoided.

The [example Makefile](Makefile) builds most of we want to set up in this class.
The default (first) target just displays some information. To actually compile the kernel
and create a `boot` directory with all the needed files, you can run

    make boot

You could then copy it over to the real boot partion with (adjust the target path to
match where your `boot` partition is mounted):

    rsync -av --delete boot/ /media/${USER}/boot/

Similarly you can build all the packages we want to have in the rootfs
(busybox, kernel modules, libc, alsa libs and mpg123) with

    make rootfs

and copy it over with

    sudo rsync -av --delete rootfs/ /media/${USER}/rootfs

Note we run this using sudo because the ext4 filesystem uses access permissions
even if we mounted it under our user. Aalternatively you could change the permissions
on the root directory after you format it, to allow write access for your user. This makes
the copying a bit more safer, thogh using sudo actually makes the files owned by root, which
is a bit "nicer" for a normal system (though the programs we have don't care about that).


The following text explains what is going on in the Makefile.


## More userspace

### libc

We don't want to compile a glibc package completely from source as all the
required libraries are already part of the crosscompiler, so we just grab them.
On ubuntu, the `gcc-arm-linux-gnueabi` package pulls in a very minimal libc
that should be OK for us. If you used crosstool-ng to compile your own
cross-compiler, it will contain a more complete libc build (already prepared
in a nice `sysroot` directory).

See the [Makefile](Makefile)

### mp3 player

We will use `mpg123`, a simple mp3 player. Even though it is simple, it still
needs the basic audio libraries (alsa libs), so we need two steps:

- cross-compile alsa libs
- cross-compile mpg123, pointing it to our cross-compiled alsa-libs


#### sysroot

TODO

#### alsa-libs

TODO see Makefile ;)

#### mpg123

TODO see Makefile ;)
