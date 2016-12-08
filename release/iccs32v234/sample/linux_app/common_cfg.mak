ARCH ?= arm64
# CROSS_COMPILE=<path_to_compiler>/bin/<toolchain_prefix>
# E.g.: CROSS_COMPILE = /home/<username>/tools/<compiler_dir>/bin/arm-none-linux-gnueabi-
# or: CROSS_COMPILE = <Yocto_BSP_path>/build_<machine>_release/tmp/sysroots/x86_64-linux/usr/bin/aarch64-fsl-linux/aarch64-fsl-linux-
# KERNELDIR = <path_to_kernel_dir>
# E.g.: KERNELDIR = <Yocto_BSP_path>/build_<machine>_release/tmp/work-shared/<machine>/kernel-build-artifacts
# for now use some yocto defaults
ifneq ($(KBUILD_OUTPUT),)
	KERNELDIR ?= $(KBUILD_OUTPUT)
endif
