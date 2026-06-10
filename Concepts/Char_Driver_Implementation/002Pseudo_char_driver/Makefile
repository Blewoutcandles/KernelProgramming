obj-m += 002Pseudo_char_driver.o

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-

KERN_DIR := /home/takeoff/code/Rspi_linux/
HOST_DIR := /lib/modules/$(shell uname -r)/build

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help
host:
	make -C $(HOST_DIR) M=$(PWD) modules

