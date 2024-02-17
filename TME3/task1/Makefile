KERNELDIR_LKP ?= $(HOME)/pnl_workplace/linux-6.5.7

obj-m += helloworld.o

PWD := $(shell pwd)

all:
	make -C $(KERNELDIR_LKP) M=$(PWD) modules

clean:
	make -C $(KERNELDIR_LKP) M=$(PWD) clean

upload:
	cp helloworld.ko $(HOME)/pnl_workplace/share/
