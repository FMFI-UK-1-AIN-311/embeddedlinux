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

### Compiling a program / library

Compiling a program or a library usually includes 3 steps (in addition to
downloading and unpacking) on linux:

- configure
- build
- install

The configure step checks (usually automatically) what features / libraries are
available on the system and the user can also specify what features they want to
enable / disable. The most popular way is to include a `configure` script
(usually created with `autoconf`), which the user just runs:

    ./configure

The actual compilation is most often handled by `make` so the second step
usually becomes:

    make -j$(nproc)

After building, the binaries might not be yet ready to just run (this depends
a lot on the program and it is setup): data files and libraries might not be in
correct locations where the program looks for them etc. Thus there is usually a
third step to install the software to a particular location (that can be
configured in the first step):

    make install


Most projects come with a default to install into `/usr/local` (which on
unix/linux is the location, where softwer installed / built manually by the
users should be placed if it should be available to everyone), which is only
writable by root. If a user just wants to install software for themselves, they
can install it into a directory inside their `home`, which would usually look
like

    ./configure --prefix=${HOME}/usr
    make -j$(nproc)
    make install

#### sysroot

When building multiple projects (libraries and programs), that depend on each
other, one has to make sure that the build system can find the dependencies,
especially header files for compilation and the library files... Normally
programs and libraries are installed in "default" system-wide locations, where
the compiler looks for them by default (i.e. `/usr/include`,
`/usr/local/include`).

When the libraries we are building are scattered through multiple build
directories, we need to tell the configure script where to find them. This can
be achieved by specifying the corresponding compiler flags (`-I` for include
file directories and `-L` for library locations). Installing all the libraries
to a single location makes it much easier (just one location to add), though it
makes it a bit harder to manage that "installation" (i.e. when removing libraries.)

Another problem arises when cross compiling: the compiler might find
headers / libraries from the system we are running on (in the default paths)
instead of the cross-compiled libraries we want. This is much harder to solve.

Modern compilers support a feature called "sysroot": we can tell them to look
for everything in a new system "root" where it will look for everything. We then
just need to create one that will contain exactly what we need.

Note: the Makefile uses intermediate directories to install packages to, and
then builds the actual sysroot by copying every required package into it
(possibly filtering out big data files that are not needed etc.) This makes it
easier to "rebuild" the sysroot with a different set of packages (i.e. when we
want to remove a library because it was causing problems, or we just messed up
something in the sysroot.)

#### alsa-libs

TODO see Makefile ;)

#### mpg123

TODO see Makefile ;)
