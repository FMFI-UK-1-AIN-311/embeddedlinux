SRC ?= src
TOOLCHAIN_DIR ?= ~/skola/embeddedlinux/cross/armv6-rpi-linux-gnueabi/bin/

export ARCH=arm
export CROSS_COMPILE=armv6-rpi-linux-gnueabi-
export PATH := $(TOOLCHAIN_DIR):$(PATH)

MAKEOPTS ?= -j$$(nproc)

OVERLAYS = base music
PACKAGES = libc alsa-lib mpg123 linux_modules

STAGE=$(PWD)/stage
SYSROOT=$(STAGE)/sysroot

FIRMWARE_FILES := bootcode.bin fixup.dat start.elf
FIRMWARE = $(foreach f,$(FIRMWARE_FILES),src/firmware/$f)

LINUX_URL=https://github.com/raspberrypi/linux/archive/rpi-5.10.y.tar.gz
LINUX_SRC=$(SRC)/$(notdir $(LINUX_URL))
LINUX_DIR=linux
LINUX_DEFCONFIG=bcmrpi_defconfig


UBOOT_GIT=git://git.denx.de/u-boot.git
UBOOT_SRC=$(SRC)/u-boot
UBOOT_DIR=u-boot
UBOOT_DEFCONFIG=rpi_0_w_defconfig

BUSYBOX_GIT=git://busybox.net/busybox.git
BUSYBOX_SRC=$(SRC)/busybox
BUSYBOX_DIR=busybox

ALSALIB_URL=http://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.5.1.tar.bz2
ALSALIB_SRC=$(SRC)/$(notdir $(ALSALIB_URL))
ALSALIB_DIR=alsa-lib-1.2.5.1

MPG123_URL=https://deac-ams.dl.sourceforge.net/project/mpg123/mpg123/1.29.1/mpg123-1.29.1.tar.bz2
MPG123_SRC=$(SRC)/$(notdir $(MPG123_URL))
MPG123_DIR=mpg123-1.29.1



# TODO this shoudl depend on "boot" and "rootfs" once those are defined
.PHONY: default
default: info

.PHONY: info
info:
	@echo "CROSS_COMPILE=$$CROSS_COMPILE"
	@echo "PATH=$$PATH"
	@echo "ARCH=$$ARCH"
	@echo "LINUX_URL=$(LINUX_URL)"
	@echo "SRC: $(SRC)"
	@echo "firmware files $(FIRMWARE)"
	@echo "MAKEOPTS=$(MAKEOPTS)"
	@echo "MAKEFLAGS=$(MAKEFLAGS)"
	@echo "Compiler version:"
	$(CROSS_COMPILE)gcc --version
	@echo

task:
	mkdir $@

$(LINUX_SRC):
	@echo "Fetching linux src $(LINUX_SRC)"
	wget -O$@ "$(LINUX_URL)"

task/do_linux_unpack: $(LINUX_SRC)
	# the rpi linux tarball contains a top directory like "linux-rpi-5.10.y" or similar
	# but we want to extract it to just $(LINUX_DIR)
	# (also tar would extract the top dir with the timestamps stored in the tar file, we want
	# it to have "current" modification time if we want to use it as a target)
	mkdir -p $(LINUX_DIR)
	tar --strip-components=1 -C $(LINUX_DIR) -xf $<
	mkdir -p task
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

task/do_linux_configure: $(LINUX_DIR)/.config
	touch "$@"

$(LINUX_DIR)/arch/arm/boot/zImage: task/do_linux_configure
	make -C $(LINUX_DIR) $(MAKEOPTS) zImage dtbs

task/do_linux_compile: $(LINUX_DIR)/arch/arm/boot/zImage
	touch "$@"

task/do_linux_modules_compile: task/do_linux_configure
	make -C $(LINUX_DIR) $(MAKEOPTS) modules
	touch "$@"

task/do_linux_modules_install: task/do_linux_modules_compile
	make -C $(LINUX_DIR) modules_install INSTALL_MOD_PATH=$(STAGE)/linux_modules
	touch "$@"

$(UBOOT_SRC)/.git:
	cd $(SRC) && git clone $(UBOOT_GIT)

task/do_uboot_fetch: $(UBOOT_SRC)/.git
	mkdir -p task
	touch "$@"

task/do_uboot_unpack: task/do_uboot_fetch
	rsync -av $(UBOOT_SRC) .
	touch "$@"

task/do_uboot_patch: task/do_uboot_unpack
	cd $(UBOOT_DIR) && patch -p1 < ../patches/u-boot-rpi0-defconfig.patch
	touch "$@"

task/do_uboot_configure: task/do_uboot_patch
	make -C $(UBOOT_DIR) $(MAKEOPTS) $(UBOOT_DEFCONFIG)
	touch "$@"

task/do_uboot_compile: task/do_uboot_configure
	make -C $(UBOOT_DIR) $(MAKEOPTS)
	touch "$@"



$(BUSYBOX_SRC)/.git:
	cd $(SRC) && git clone $(BUSYBOX_GIT)

task/do_busybox_fetch: $(BUSYBOX_SRC)/.git
	mkdir -p task
	touch "$@"

task/do_busybox_unpack: task/do_busybox_fetch
	rsync -av $(BUSYBOX_SRC) .
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
# boot
#

task/do_boot: task/do_linux_compile task/do_uboot_compile $(FIRMWARE)
	rm -rf boot
	mkdir boot
	cp $(LINUX_DIR)/arch/arm/boot/zImage boot
	cp $(LINUX_DIR)/arch/arm/boot/dts/*rpi*.dtb boot
	# HACK use the rpi foundation device tree
	# otherwise usb doesn't work
	cp boot/bcm2708-rpi-zero-w.dtb boot/bcm2835-rpi-zero-w.dtb

	cp $(UBOOT_DIR)/u-boot.bin  boot/kernel.img

	cp $(FIRMWARE) boot
	echo 'enable_uart=1' >>boot/config.txt
	touch "$@"

.PHONY: boot
boot: task/do_boot


task/do_libc_install:
	rsync -av --exclude="locale-archive" $(TOOLCHAIN_DIR)/../armv6-rpi-linux-gnueabi/sysroot/ \
		$(STAGE)/libc
	# fix weird sysroot dirs not writable
	chmod -R u+w $(STAGE)/libc
	mkdir -p task && touch "$@"

task/do_libc_stage: task/do_libc_install
	mkdir -p $(SYSROOT)
	rsync -av $(STAGE)/libc/ $(SYSROOT)/
	mkdir -p task && touch "$@"


$(ALSALIB_SRC):
	wget -O$(ALSALIB_SRC) "$(ALSALIB_URL)"
	touch "$@"

task/do_alsa-lib_unpack: $(ALSALIB_SRC)
	tar xf $(ALSALIB_SRC)
	touch "$@"

task/do_alsa-lib_configure: task/do_alsa-lib_unpack
	cd $(ALSALIB_DIR) && ./configure --host=armv6-rpi-linux-gnueabi --prefix=/usr
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

$(MPG123_SRC):
	wget -O$(MPG123_SRC) "$(MPG123_URL)"
	touch "$@"

task/do_mpg123_unpack: $(MPG123_SRC)
	tar xf $(MPG123_SRC)
	touch "$@"

task/do_mpg123_configure: task/do_mpg123_unpack task/do_alsa-lib_stage
	cd $(MPG123_DIR) && \
		LDFLAGS="--sysroot=$(SYSROOT)" CFLAGS="--sysroot=$(SYSROOT)" \
		./configure --host=armv6-rpi-linux-gnueabi --prefix=/usr \
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



.PHONY: rootfs
rootfs: task/do_busybox_install $(foreach o,$(OVERLAYS),overlays/$o) $(foreach p,$(PACKAGES),task/do_$(p)_install)
	rm -rf rootfs
	mkdir rootfs
	rsync -av $(BUSYBOX_DIR)/_install/ rootfs/
	touch "$@"
	for p in $(PACKAGES) ; do \
		rsync -av --exclude="*.h" --exclude="*.a" $(STAGE)/$$p/ rootfs/ ; \
	done
	for o in $(OVERLAYS) ; do \
		rsync -av overlays/$$o/ rootfs/ ; \
	done

# TODO u-boot..
# you can eiter fetch/unpack a tarball of a recent release like with kernel.. or for a git checkout:
# - instead of fetching http you can "git clone" direclty into SRC, so you have a "clean" download/checkout
# - instead of unpacking, you can copy / "rsync -a" the clone from src into the final dir...

# TODO build busybox, install...
# TODO fetch firmware files into src
# TODO create a boot directory and copy everything needed into it
#      (fw files, kernel, kernel dtbs, u-boot, create config.txt)

# TODO create a rootfs dir with everything we want in rootfs:
#   - copy over busybox/_install
#   - create dirs for dev, proc, sys
#   - copy over any custom files (it's best to keep them in as full tree  a direcotry and rsync them into the final rootfs)
#     - /etc/init.d/rcS for the default startup script, this should at least
#       mount /proc and /sys and remount root as read-write
#     - any other custom configs...

# ADVANCED: find how you can modify the default "bootcmd" and "bootargs" envvars in u-boot so you can build it directly
# with the values we want, incorporate it into the "make recipe" you created

