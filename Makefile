obj-m := 001Helloworld.o

KDIR := /home/takeoff/code/Rspi_linux/
HOST_KDIR := /lib/modules/$(shell uname -r)/build

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) help
host:
	make -C $(HOST_KDIR) M=$(PWD) modules