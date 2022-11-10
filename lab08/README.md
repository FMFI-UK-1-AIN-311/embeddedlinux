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
**TODO this is from last year, it builds also u-boot, which we don't use,
and expects a different crosscompiler (libc paths need to be changed).**



## More userspace

### libc

### mp3 player

- `alsalib` libraries (for sound output)
- `mpg123` program
