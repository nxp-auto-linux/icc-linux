.PHONY:kbuild
PWD := $(shell pwd)

# contains host specific definitions
include ../common_cfg.mak

kbuild: copy
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=1 -C $(KERNELDIR) M=$(PWD)

modules_install: kbuild
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH="$(DESTDIR)"

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) clean
	-rm -f *~ Module.symvers Module.markers modules.order
	find . -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" | xargs rm -f
	find $(PWD)/../../config/src -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" | xargs rm -f
	#rm -rf ./.tmp_versions

copy:
	@if [ ! -f "../ICC_module/Module.symvers" ]; then \
		echo '../ICC_module/Module.symvers not found.'; \
		$(MAKE) -C $(PWD)/../ICC_module KERNEL_SRC=$(KERNEL_SRC); \
	fi;
	cp ../ICC_module/Module.symvers .

.PHONY: clean
