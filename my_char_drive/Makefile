CONFIG_MODULE_SIG=n

MODULE_NAME := my_char_drive

ifneq ($(KERNELRELEASE),)

obj-m := $(MODULE_NAME).o

else
CURRENT_PATH ?= $(shell pwd)
LINUX_KERNEL ?= $(shell uname -r)
LINUX_KERNEL_PATH ?= /lib/modules/$(LINUX_KERNEL)/build

CURRENT_PATH := $(shell pwd)

modules:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules

modules_install:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules_install

insmod:
	sudo insmod $(MODULE_NAME).ko
rmmod:
	sudo rmmod $(MODULE_NAME)

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm -f modules.order Module.symvers Module.markers

out: rmmod clean
	sudo dmesg -C
	rm -f client

in: modules insmod

client:
	gcc -o client client.c -lpthread

endif