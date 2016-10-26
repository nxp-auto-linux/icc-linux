.PHONY:kbuild
PWD := $(shell pwd)

# contains host specific definitions
include ../common_cfg.mak

kbuild: copy
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=1 -C $(KERNELDIR) M=$(PWD)

modules_install: kbuild
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH="$(DESTDIR)"
	@echo Copying ICC headers to toolchain
	mkdir -p "$(DESTDIR)/usr/include/linux"
	cp -f "$(PWD)/../../../code/linux/inc/ICC_Api.h" "$(DESTDIR)/usr/include/linux/"

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) clean
	-rm -f *~ Module.symvers Module.markers modules.order
	find . -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" -o -name "*.ko" | xargs rm -f
	find $(PWD)/../ICC_Sample_module_common -name "*.cmd" -o -name "*.o" | xargs rm -f
	find $(PWD)/../../config/src -name "*.cmd" -o -name "*.o" | xargs rm -f
	rm -rf $(PWD)/../ICC_Sample_module_common/.tmp_versions

copy:
	@if [ ! -f "../ICC_module_EP/Module.symvers" ]; then \
		echo '../ICC_module_EP/Module.symvers not found.'; \
		$(MAKE) -C $(PWD)/../ICC_module_EP KERNEL_SRC=$(KERNEL_SRC); \
	fi;
	cp ../ICC_module_EP/Module.symvers .

.PHONY: clean
