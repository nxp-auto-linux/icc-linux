.PHONY:kbuild
PWD := $(shell pwd)

# contains host specific definitions
include ../common_cfg.mak

kbuild: copy
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=1 -C $(KERNELDIR) M=$(PWD)

modules_install: kbuild
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH="$(DESTDIR)"
	@echo Copying mcc headers to toolchain
	mkdir -p "$(DESTDIR)/usr/include/linux"
	cp -f "$(PWD)/../../../code/linux/inc/ICC_Api.h" "$(DESTDIR)/usr/include/linux/"
	mkdir -p "$(DESTDIR)/opt/AUTOSAR"
	cp -f "$(PWD)/../../autosar_app/icc_sample_asr.bin" "$(DESTDIR)/opt/AUTOSAR"

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) clean
	-rm -f *~ Module.symvers Module.markers modules.order
	find . -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" -o -name "*.ko" | xargs rm -f

copy:
	@if [ ! -f "../ICC_module/Module.symvers" ]; then \
		echo '../ICC_module/Module.symvers not found.'; \
		$(MAKE) -C $(PWD)/../ICC_module KERNEL_SRC=$(KERNEL_SRC); \
	fi;
	cp ../ICC_module/Module.symvers .

.PHONY: clean
