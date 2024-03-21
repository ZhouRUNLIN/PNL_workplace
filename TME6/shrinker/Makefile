KERNELDIR_LKP ?= $(HOME)/pnl_workplace/linux-6.5.7
SHAREDDIR_LKP ?= $(HOME)/pnl_workplace/share

obj-m += shrinker.o

all: clean app upload

app :
	make -C $(KERNELDIR_LKP) M=$$PWD modules

clean:
	make -C $(KERNELDIR_LKP) M=$$PWD clean

upload:
	cp shrinker.ko $(SHAREDDIR_LKP)