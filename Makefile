KVERSION := `uname -r`
KDIR := /lib/modules/${KVERSION}/build

default: clean
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

install: default
	$(MAKE) -C $(KDIR) M=$$PWD modules_install
	depmod -A

unload:
	rmmod hid_gx100_shifter;

load:
	insmod hid_gx100_shifter.ko