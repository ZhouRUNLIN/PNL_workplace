KERNELDIR_LKP ?= /lib/modules/$(shell uname -r)/build

obj-m += kcpustat.o

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR_LKP) M=$$PWD modules

clean:
	make -C $(KERNELDIR_LKP) M=$$PWD clean
