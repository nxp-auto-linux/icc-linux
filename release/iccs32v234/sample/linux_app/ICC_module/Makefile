.PHONY:kbuild
PWD := $(shell pwd)

# contains host specific definitions
include ../common_cfg.mak

kbuild:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=1 -C $(KERNELDIR) M=$(PWD)

modules_install: kbuild
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=$(DESTDIR)

headers_install:
	@echo Copying ICC headers to filesystem
	mkdir -p "$(DESTDIR)/usr/include/linux"
	cp -f "$(PWD)/../../../code/linux/inc/ICC_Api.h" "$(DESTDIR)/usr/include/linux/"

m4_install:
	@echo Copying M4 binary to filesystem
	mkdir -p "$(DESTDIR)/opt/AUTOSAR"
	cp -f "$(PWD)/../../autosar_app/icc_sample_asr.bin" "$(DESTDIR)/opt/AUTOSAR"

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)  -C $(KERNELDIR) M=$(PWD) clean
	find $(PWD)/../../../code -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" | xargs rm -f
	find . -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" | xargs rm -f
	#rm -rf ./.tmp_versions
