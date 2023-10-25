More userspace or compiling programs and libraries
==================================================

Compiling busybox was fine (and it gave us a fully functional small linux
system,) but we would like also include some more interesting programs.

To do that, we will need

- to know how to build programs and libraries on linux in general,
- get the (dynamic) C libraries on the device, so we don't have to build
  everything as static binaries.


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

#### CMake

CMake is a (relatively newer) build tool for (mostly, but no only) C/C++ applications.
While it uses the same 3 steps, the invocations are slightly different:

    # configure a cmake build (inside a different "build" direcotry
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${HOME}/usr ..  # the .. argument is path to the sources

    # build, the default "gnerator" in cmake is Makefiles, so we build with make
    make -j$(nproc)

    # install
    make install

### An mp3 player

We will use the `mpg123` program, a simple mp3 player, as an example. First
let's try to build it for our computer.

Check out [mpg123's homepage](https://www.mpg123.de/) and download the source
code

    wget https://deac-ams.dl.sourceforge.net/project/mpg123/mpg123/1.32.3/mpg123-1.32.3.tar.bz2

and unpack it

    tar xf mpg123-1.32.3.tar.bz2

then change into the unpacked directory

    cd mpg123-1.32.3

You could check the configure options `./configure --help`, for now will go with
the defaults nad only change the install location ("prefix")

    ./configure --prefix=$PWD/../mpg123-install/

Compile it

    make -j$(nproc)

You can find the binary in `src/.libs`, but we can properly "instlal" it into
the directory we specified with the `--prefix` parameter before by running

    make install

You can check what was installed there with

    find ../mpg123-install

Now we can try to run it

    ../mpg123-install/bin/mpg123
../mpg123-install/bin/mpg123: error while loading shared libraries: libout123.so.0: cannot open shared object file: No such file or directory

Oops.. looks like it would some dynamic libraries:

    ldd ../mpg123-install/bin/mpg123

            linux-vdso.so.1 (0x00007fffc69de000)
            libmpg123.so.0 => /lib/x86_64-linux-gnu/libmpg123.so.0 (0x00007ff497ed1000)
            libout123.so.0 => not found
            libsyn123.so.0 => not found

Remeber the libraries in `../mpg123-install/lib`? We need to tell the
dynamic loader to look there

    export LD_LIBRARY_PATH=$PWD/../mpg123-install/lib

Now we can try again (you can also check with `ldd` as before)

    ../mpg123-install/bin/mpg123

    You made some mistake in program usage... let me briefly remind you:
    
    High Performance MPEG 1.0/2.0/2.5 Audio Player for Layers 1, 2 and 3
            version 1.32.3; written and copyright by Michael Hipp and others
            free software (LGPL) without any warranty but with best wishes
    
    usage: mpg123 [option(s)] [file(s) | URL(s) | -]

Great! Now just grab some mp3s and really test it ;)


## Compiling mpg123 for RPi

Let's compile mpg123 for the raspberry.
Even though it is simple, it still needs the basic audio libraries (alsa libs),
so we will need to compile two things:

- cross-compile alsa libs
- cross-compile mpg123, pointing it to our cross-compiled alsa-libs

Oh, and because we don't want to mess with static binaries anymore, we'll need
to get some `libc` libraries on the device...

But first we need to talk about how the compiler finds the right headers and
libraries when cross-compiling...


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

Note: automaged build systems use intermediate directories to install packages
to, and then build the actual sysroot by copying every required package into it
(possibly filtering out big data files that are not needed etc.) This makes it
easier to "rebuild" the sysroot with a different set of packages (i.e. when we
want to remove a library because it was causing problems, or we just messed up
something in the sysroot.)

We'll use the variables STAGE and SYSROOT to point to two directories, which
we'll use as the staging and sysroot directories (the staging dir will have
separate subdirecotries for each "package".)

    STAGE=$PWD/stage
    SYSROOT=$PWD/sysroot

Let's create them upfront

    mkdir -p ${STAGE?}
    mkdir -p ${SYSROOT?}

### busybox

We will still need busybox in the final filesystem, so let's put it into the
sysroot. We don't need it for the compilation, but we will be able to use the
sysroot as the rootfs directly. The following command assumes you built busybox
in the `busybox-1.36.1/` folder (and it includes the `_install` folder)

    mkdir -p ${STAGE?}/busybox
    rsync -av busybox-1.36.1/_install/ ${STAGE?}/busybox/

And put it into the sysroot

    rsync -av ${STAGE?}/busybox/ ${SYSROOT?}/


### libc

We don't want to compile a glibc package completely from source as all the
required libraries are already part of the crosscompiler, so we just grab them.
On ubuntu, the `gcc-arm-linux-gnueabi` package pulls in a very minimal libc
that should be OK for us. If you used crosstool-ng to compile your own
cross-compiler, it will contain a more complete libc build (already prepared
in a nice `sysroot` directory, see above).

TODO

```sh
TOOLCHAIN_DIR=/usr/arm-linux-gnueabi
mkdir -p ${STAGE?}/libc
# copy everything except include (which we'll copy later), bin, which contains
# x86 binaries and and locale-archive # which we don't want right now (it's
# a big locale information database)
rsync -av --exclude="locale-archive" --exclude=include --exclude=bin --copy-unsafe-links ${TOOLCHAIN_DIR?}/ ${STAGE?}/libc/
# we want include under /usr
mkdir -p ${STAGE?}/libc/usr
rsync -av ${TOOLCHAIN_DIR?}/include ${STAGE?}/libc/usr/
```

Note: we want `include` in the sysroot, so we can compile against it, but we
don't need it on the devce to run programs...

And now put it into the sysroot

    rsync -av ${STAGE?}/libc/ ${SYSROOT?}/

### alsa-libs

TODO host triplets, --build --host...

    HOST=arm-linux-gnueabi




    wget http://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.5.1.tar.bz2
    tar xf alsa-lib-1.2.5.1.tar.bz2
    cd alsa-lib-1.2.5.1/

    LDFLAGS="--sysroot=${SYSROOT?}" CFLAGS="--sysroot=${SYSROOT?}" \
        ./configure --host=${HOST?} --prefix=/usr


TODO make install DESTDIR

    make -j$(nproc) install DESTDIR=${STAGE?}/alsa-lib

And put it into the sysroot, so that following programs can compile against it

    rsync -av ${STAGE?}/alsa-lib/ ${SYSROOT?}/

Finished in alsa-libs, we can move out of that directory ;)

    cd ..

### mpg123

First, remove the old compilation of mpg123 for your computer if you are doing
this in the same directory :).

    wget https://deac-ams.dl.sourceforge.net/project/mpg123/mpg123/1.32.3/mpg123-1.32.3.tar.bz2
    tar xf mpg123-1.32.3.tar.bz2
    cd mpg123-1.32.3

This time we will run mpg123's configure with an argument specifying that we
wnat just the alsa audio backend (in addition to the options needed for
cross-compilation

    LDFLAGS="--sysroot=${SYSROOT?}" CFLAGS="--sysroot=${SYSROOT?}" \
        ./configure --host=${HOST?} --prefix=/usr \
        --with-audio=alsa

    make -j$(nproc)
    make install DESTDIR=${STAGE?}/mpg123

And copy over to the sysroot

    rsync -av ${STAGE?}/mpg123/ ${SYSROOT?}/

Finished in mpg123, we can move out of that directory ;)

    cd ..

### alsa-utils

The alsa-libs "package" contains only the libraries needed to write programs,
but the alsa project also has some handy tools (volume mixer, simple wav
player/recorder etc.) These are in a separate package called alsa-utils.
You can download and build them similarly to previous two items.

    wget http://www.alsa-project.org/files/pub/utils/alsa-utils-1.2.5.1.tar.bz2
    ...

### modules

To be able 

In the kernel build (don't forget to set `ARCH` and `CROSS_COMPILE` correctly)
run (this will take moooore time than the kernel itself):

    make -j$(nproc) modules

Then install them to the staging dir

	make modules_install INSTALL_MOD_PATH=${STAGE?}/linux_modules

And finaly install them into sysroot

    rsync -av ${STAGE?}/linux_modules/ ${SYSROOT?}/

### Audio group for alsa library

The alsa library expects an `audio` group to be configured, which is done ine a `/etc/group` file, but we have none for now..

    mkdir ${SYSROOT?}/etc
    echo 'audio:x:1000:' > ${SysROOT?}/etc/group

### Final filesystem

We can now copy over the final filesystem (sysroot) to the rootfs on the sd card.
Note that we could ommit unneeded files (such as the c includes,) but it doesn't
ammount to too much space right now.

    sudo rsync -av ${SYSROOT?}/ /media/${USER}/rootfs/

**Maybe put an mp3 file on the device so we can actually test it ;)**

### Actually run it

TODO ;)))

load soundcard module...
(if we don't have real sound cards, because I forgot them, load `snd-dummy`)

    modprobe snd_usb_audio

