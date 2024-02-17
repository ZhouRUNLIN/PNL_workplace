KERNELDIR_LKP ?= /lib/modules/$(shell uname -r)/build

obj-m += prdebug.o

all :
	make -C $(KERNELDIR_LKP) M=$$PWD modules

clean:
	make -C $(KERNELDIR_LKP) M=$$PWD clean
