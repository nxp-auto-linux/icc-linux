.PHONY:kbuild
PWD := $(shell pwd)

# contains host specific definitions
include ../common_cfg.mak

kbuild:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=1 -C $(KERNELDIR) M=$(PWD) 

install: kbuild
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=$(DESTDIR)

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)  -C $(KERNELDIR) M=$(PWD) clean
	find ../../../../code -name "*.cmd" -o -name "*.o" | xargs rm -f
