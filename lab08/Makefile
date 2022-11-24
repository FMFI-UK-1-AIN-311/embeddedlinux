SRC ?= src
TOOLCHAIN_DIR ?= /usr/arm-linux-gnueabi

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
export HOST=$(CROSS_COMPILE:-=)

MAKEOPTS ?= -j$$(nproc)


OVERLAYS = base music
PACKAGES = libc alsa-lib alsa-utils mpg123 linux_modules

STAGE=$(PWD)/stage
SYSROOT=$(STAGE)/sysroot

FIRMWARE_FILES := bootcode.bin fixup.dat start.elf
FIRMWARE = $(foreach f,$(FIRMWARE_FILES),src/firmware/$f)

LINUX_URL=https://github.com/raspberrypi/linux/archive/rpi-6.1.y.tar.gz
LINUX_SRC=$(SRC)/$(notdir $(LINUX_URL))
LINUX_DIR=linux
LINUX_DEFCONFIG=bcmrpi_defconfig

BUSYBOX_URL=https://busybox.net/downloads/busybox-1.35.0.tar.bz2
BUSYBOX_SRC=$(SRC)/$(notdir $(BUSYBOX_URL))
BUSYBOX_DIR=$(notdir $(BUSYBOX_SRC:.tar.bz2=))

ALSALIB_URL=http://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.5.1.tar.bz2
ALSALIB_SRC=$(SRC)/$(notdir $(ALSALIB_URL))
ALSALIB_DIR=$(notdir $(ALSALIB_SRC:.tar.bz2=))

ALSAUTILS_URL=http://www.alsa-project.org/files/pub/utils/alsa-utils-1.2.5.1.tar.bz2
ALSAUTILS_SRC=$(SRC)/$(notdir $(ALSAUTILS_URL))
ALSAUTILS_DIR=$(notdir $(ALSAUTILS_SRC:.tar.bz2=))


MPG123_URL=https://deac-ams.dl.sourceforge.net/project/mpg123/mpg123/1.29.1/mpg123-1.29.1.tar.bz2
MPG123_SRC=$(SRC)/$(notdir $(MPG123_URL))
MPG123_DIR=$(notdir $(MPG123_SRC:.tar.bz2=))



# TODO this shoudl depend on "boot" and "rootfs" once those are defined
.PHONY: default
default: info

.PHONY: info
info:
	@echo "ARCH=$$ARCH"
	@echo "CROSS_COMPILE=$$CROSS_COMPILE"
	@echo "HOST=$(HOST)"
	@echo "PATH=$$PATH"
	@echo "SRC=$(SRC)"
	@echo "MAKEOPTS=$(MAKEOPTS)"
	@echo "MAKEFLAGS=$(MAKEFLAGS)"
	@echo
	@echo "Compiler version: \t$$($(CROSS_COMPILE)gcc --version | head -n1)"
	@echo
	@echo "Firmware files: \t$(FIRMWARE)"
	@echo
	@echo "LINUX_URL=$(LINUX_URL)"
	@echo "LINUX_DIR=$(LINUX_DIR)"
	@echo "LINUX_DEFCONFIG=$(LINUX_DEFCONFIG)"
	@echo
	@echo "BUSYBOX_URL=$(BUSYBOX_URL)"
	@echo "BUSYBOX_DIR=$(BUSYBOX_DIR)"
	@echo
	@echo "ALSALIB_URL=$(ALSALIB_URL)"
	@echo "ALSALIB_DIR=$(ALSALIB_DIR)"
	@echo
	@echo "MPG123_URL=$(MPG123_URL)"
	@echo "MPG123_DIR=$(MPG123_DIR)"
	@echo
	@echo
	@echo "Run"
	@echo
	@echo "    make boot rootfs"
	@echo
	@echo "to compile everything and create boot and rootfs directories to be"
	@echo "copied over to SD card."

task/.dir:
	mkdir -p $(dir $@)

$(LINUX_SRC):
	@echo "Fetching linux src $(LINUX_SRC)"
	mkdir -p $(dir $@)
	wget -O$@ "$(LINUX_URL)"

task/do_linux_unpack: $(LINUX_SRC) task/.dir
	# the rpi linux tarball contains a top directory like "linux-rpi-5.10.y" or similar
	# but we want to extract it to just $(LINUX_DIR)
	# (also tar would extract the top dir with the timestamps stored in the tar file, we want
	# it to have "current" modification time if we want to use it as a target)
	mkdir -p $(LINUX_DIR)
	tar --strip-components=1 -C $(LINUX_DIR) -xf $(LINUX_SRC)
	touch "$@"

# When we want to run make in the subdirs to build kernel/..., it is a bit questionable whether
# we want to use the standard way to write "recursion into subdirs" in make [1].
# - $(MAKE) -C subdir $(MAKEFLAGS)
#   this is the usual make idiom to recurse into subdirectories with exactly the same
#   options that make was invoked with, sharing the "-j" job number limitations and
#   doing some special handling for options like -t and -k.
#   If we had rules to build kernel an u-boot this way and executed `make -j12`, then
#   make might run both builds in parallel with a "global" max of 12 threads...
# - make -C subdir $(MAKEOPTS) [or other "fixed" options]
#   this sort of treats the "sub" make as any other build command without special
#   handling. The MAKEOPTS variable isn't special in any way, just to have a single definition
#   for all our make invocations...
#   Normally we would set MAKEOPTS=-j$$(nproc) and run just `make` ("single threaded") on the top level,
#   but then the sub makes will be invoked with say -j12... Of course if we ran `make -j2` we could
#   have 12 threads building linux and 12 threads building u-boot...
#
# [1] https://www.gnu.org/software/make/manual/html_node/Recursion.html

$(LINUX_DIR)/.config: task/do_linux_unpack
	make -C $(LINUX_DIR) $(MAKEOPTS) $(LINUX_DEFCONFIG)

task/do_linux_configure: task/.dir $(LINUX_DIR)/.config
	touch "$@"

$(LINUX_DIR)/arch/arm/boot/zImage: task/do_linux_configure
	make -C $(LINUX_DIR) $(MAKEOPTS) zImage dtbs

task/do_linux_compile: task/.dir $(LINUX_DIR)/arch/arm/boot/zImage
	touch "$@"

task/do_linux_modules_compile: task/do_linux_configure
	make -C $(LINUX_DIR) $(MAKEOPTS) modules
	touch "$@"

task/do_linux_modules_install: task/do_linux_modules_compile
	make -C $(LINUX_DIR) modules_install INSTALL_MOD_PATH=$(STAGE)/linux_modules
	touch "$@"


#
# busybox
#
$(BUSYBOX_SRC):
	mkdir -p $(dir $@)
	wget -O$(BUSYBOX_SRC) "$(BUSYBOX_URL)"
	touch "$@"

task/do_busybox_unpack: $(BUSYBOX_SRC) task/.dir
	tar xf $(BUSYBOX_SRC)
	touch "$@"

task/do_busybox_configure: task/do_busybox_unpack
	make -C $(BUSYBOX_DIR) $(MAKEOPTS) defconfig
	sed -i -e 's,.*CONFIG_BUSYBOX_EXEC_PATH=.*,CONFIG_BUSYBOX_EXEC_PATH="/bin/busybox",' \
		$(BUSYBOX_DIR)/.config
	sed -i -e "s,# CONFIG_STATIC.*,CONFIG_STATIC=y," \
		$(BUSYBOX_DIR)/.config
	touch "$@"

task/do_busybox_compile: task/do_busybox_configure
	make -C $(BUSYBOX_DIR) $(MAKEOPTS)
	touch "$@"

task/do_busybox_install: task/do_busybox_compile
	make -C $(BUSYBOX_DIR) $(MAKEOPTS) install
	touch "$@"


#
# firmware
#
src/firmware/%:
	mkdir -p $(dir $@)
	cd $(dir $@) && wget https://github.com/raspberrypi/firmware/raw/master/boot/$*


#
# Applications and other parts that will be installed to rootfs.
# The targets generally follow  following "conventions"
#
# - task/do_<PACKAGE_NAME>_install: install files (usually via packages 'make install' or similar)
#   to a per-package directory $(STAGE)/<PACKAGE_NAME>
#
# - task/do_<PACKAGE_NAME>_stage: move (possibly subset of) the files from the install location
#   to the combined $(STAGE)/sysroot
#

#
# libc
#
# We just copy libc libraries from the compiler instead of downloading and building
# a full glibc package.
# The following is specific to ubuntu's arm crosscompiler setup
task/do_libc_install: task/.dir
	mkdir -p $(STAGE)/libc/usr/include
	rsync -av --exclude="locale-archive" --exclude=include --exclude=bin --copy-unsafe-links $(TOOLCHAIN_DIR)/ $(STAGE)/libc/
	# bin/ contains just symlinks to /usr/bin
	rsync -avL $(TOOLCHAIN_DIR)/bin $(STAGE)/libc/
	# we want include under /usr
	rsync -av $(TOOLCHAIN_DIR)/include $(STAGE)/libc/usr/
	# TODO /usr/bin/ld.so ????
	touch "$@"

task/do_libc_stage: task/do_libc_install
	mkdir -p $(SYSROOT)
	rsync -av $(STAGE)/libc/ $(SYSROOT)/
	touch "$@"


#
# alsalib (audio library)
#
$(ALSALIB_SRC):
	mkdir -p $(dir $@)
	wget -O$(ALSALIB_SRC) "$(ALSALIB_URL)"
	touch "$@"

task/do_alsa-lib_unpack: task/.dir $(ALSALIB_SRC)
	tar xf $(ALSALIB_SRC)
	touch "$@"

task/do_alsa-lib_configure: task/do_alsa-lib_unpack 
	cd $(ALSALIB_DIR) && \
		LDFLAGS="--sysroot=$(SYSROOT)" CFLAGS="--sysroot=$(SYSROOT)" \
		./configure --host=$(HOST) --prefix=/usr
	touch "$@"

task/do_alsa-lib_compile: task/do_alsa-lib_configure task/do_libc_stage
	make -C $(ALSALIB_DIR) $(MAKEOPTS)
	touch "$@"

task/do_alsa-lib_install: task/do_alsa-lib_compile
	make -C $(ALSALIB_DIR) install DESTDIR=$(STAGE)/alsa-lib
	touch "$@"

task/do_alsa-lib_stage: task/do_alsa-lib_install
	rsync -av $(STAGE)/alsa-lib/ $(SYSROOT)/
	touch "$@"

#
# alsa utils
#
$(ALSAUTILS_SRC):
	mkdir -p $(dir $@)
	wget -O$(ALSAUTILS_SRC) "$(ALSAUTILS_URL)"
	touch "$@"

task/do_alsa-utils_unpack: task/.dir $(ALSAUTILS_SRC)
	tar xf $(ALSAUTILS_SRC)
	touch "$@"

task/do_alsa-utils_configure: task/do_alsa-utils_unpack task/do_alsa-lib_stage
	cd $(ALSAUTILS_DIR) && \
		LDFLAGS="--sysroot=$(SYSROOT)" CFLAGS="--sysroot=$(SYSROOT)" \
		./configure --host=$(HOST) --prefix=/usr --disable-alsamixer --disable-alsaconf
	touch "$@"

task/do_alsa-utils_compile: task/do_alsa-utils_configure task/do_libc_stage
	make -C $(ALSAUTILS_DIR) $(MAKEOPTS) aplay
	touch "$@"

task/do_alsa-utils_install: task/do_alsa-utils_compile
	make -C $(ALSAUTILS_DIR) install DESTDIR=$(STAGE)/alsa-utils
	touch "$@"

task/do_alsa-utils_stage: task/do_alsa-utils_install
	rsync -av $(STAGE)/alsa-utils/ $(SYSROOT)/
	touch "$@"

#
# mpg123 (audio player)
#
$(MPG123_SRC):
	mkdir -p $(dir $@)
	wget -O$(MPG123_SRC) "$(MPG123_URL)"
	touch "$@"

task/do_mpg123_unpack: task/.dir $(MPG123_SRC)
	tar xf $(MPG123_SRC)
	touch "$@"

task/do_mpg123_configure: task/do_mpg123_unpack task/do_alsa-lib_stage
	cd $(MPG123_DIR) && \
		LDFLAGS="--sysroot=$(SYSROOT)" CFLAGS="--sysroot=$(SYSROOT)" \
		./configure --host=$(HOST) --prefix=/usr \
		--with-audio=alsa
	touch "$@"

task/do_mpg123_compile: task/do_mpg123_configure task/do_libc_stage
	make -C $(MPG123_DIR) $(MAKEOPTS)
	touch "$@"

task/do_mpg123_install: task/do_mpg123_compile
	make -C $(MPG123_DIR) install DESTDIR=$(STAGE)/mpg123
	touch "$@"

task/do_mpg123_stage: task/do_mpg123_install
	rsync -av $(STAGE)/mpg123/ $(SYSROOT)/
	touch "$@"


#
# "base" filesystem structure: basic dirs and a very basic startup script
#
# With the default configuration (nonexistent /etc/inittab, which will casue busybox
# to use the build in one) busybox will run /etc/init.d/rcS at startup.
# See https://github.com/brgl/busybox/blob/master/examples/inittab#L35-L45
define rcSContents
#!/bin/sh

echo "Mounting filesystems"
mount -t proc proc /proc
mount -t sysfs sys /sys
mount -t tmpfs tmpfs /tmp
mount -t tmpfs tmpfs /var

echo "Re-mounting / rw"
mount -o remount,rw /

echo "Initializing mdev"
echo /sbin/mdev > /proc/sys/kernel/hotplug
echo "Initialize devices"
mdev -s

echo "Loading audio driver"
modprobe snd-usb-audio

# Note: uncomment this to automatically start music playback ;)
#echo "Wait a bit to allow the usb devie to appear/initialize"
#sleep 5
#
#echo "Playing some music"
#mpg123 /usr/share/music/* &
endef
export rcSContents

overlays/base/etc/init.d/rcS:
	mkdir -p $(dir $@)
	echo "$$rcSContents" > $@
	chmod 755 $@

overlays/base: overlays/base/etc/init.d/rcS
	bash -c 'mkdir -p $@/{dev/pts,etc/init.d,mnt,proc,sys,tmp,var}'

#
# music overlay
#
# Create just put some mp3 in here...
overlays/music:
	mkdir -p $@


#
# boot
#
.PHONY: boot
boot: task/do_linux_compile $(FIRMWARE)
	rm -rf boot # we just always re-create the boot dir
	mkdir boot
	# firmware
	cp $(FIRMWARE) boot
	# kernel
	cp $(LINUX_DIR)/arch/arm/boot/zImage boot/kernel.img
	cp $(LINUX_DIR)/arch/arm/boot/dts/*.dtb boot
	mkdir boot/overlays
	cp -ar $(LINUX_DIR)/arch/arm/boot/dts/overlays/*.dtb boot/overlays
	cp -ar $(LINUX_DIR)/arch/arm/boot/dts/overlays/*.dtbo boot/overlays
	# config.txt and cmdline.txt
	echo "dtoverlay=miniuart-bt" > boot/config.txt
	echo "8250.nr_uarts=1 earlyprintk console=ttyAMA0,115200 root=/dev/mmcblk0p2 rootwait init=/sbin/init" > boot/cmdline.txt


#
# rootfs
#
# Copy over packages and the overlays to the final rootfs dir.
.PHONY: rootfs
rootfs: task/do_busybox_install $(foreach o,$(OVERLAYS),overlays/$o) $(foreach p,$(PACKAGES),task/do_$(p)_install)
	rm -rf rootfs # we just always re-create the rootfs dir
	mkdir rootfs
	rsync -av $(BUSYBOX_DIR)/_install/ rootfs/
	touch "$@"
	for p in $(PACKAGES) ; do \
		rsync -av --exclude="*.h" --exclude="*.a" $(STAGE)/$$p/ rootfs/ ; \
	done
	for o in $(OVERLAYS) ; do \
		rsync -av overlays/$$o/ rootfs/ ; \
	done
