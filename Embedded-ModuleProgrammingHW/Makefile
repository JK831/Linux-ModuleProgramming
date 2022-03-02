CONFIG_MODULE_SIG=n

obj-m += MyBufferedMem.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf MyBufferedMem.o MyBufferedMem.mod.* MyBufferedMem.ko
