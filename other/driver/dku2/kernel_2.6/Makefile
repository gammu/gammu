ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
EXTRA_CFLAGS += -Idrivers/usb/serial

obj-$(CONFIG_USB_SERIAL_NOKIA_DKU2) := nokia_dku2.o
else

# Normal Makefile

KERNELDIR := /lib/modules/`uname -r`/build
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` CONFIG_USB_SERIAL_NOKIA_DKU2=m modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules_install
clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

endif

