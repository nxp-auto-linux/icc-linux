.PHONY:kbuild
PWD := $(shell pwd)

# contains host specific definitions
include ../common_cfg.mak

kbuild: copy
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=1 -C $(KERNELDIR) M=$(PWD) 

install: kbuild
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=$(DESTDIR)
	@echo Copying mcc headers to toolchain
	mkdir -p $(DESTDIR)/usr/include/linux
	cp -f $(PWD)/../../../code/inc/ICC_Api.h $(DESTDIR)/usr/include/linux/

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) clean
	-rm -f *~ Module.symvers Module.markers modules.order
	find . -name "*.cmd" -o -name "*.o" -o -name "*.mod.c" -o -name "*.ko" | xargs rm -f

copy:
	cp ../ICC_module/Module.symvers .

.PHONY: clean
