
ROOT=..
MODULE_NAME := force_rmmod
EXTRA_CFLAGS  += -g -std=gnu99  -Wfatal-errors 

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

reinsmod:
	sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

github:
	cd $(ROOT) && make github

rmmod:
	sudo rmmod $(MODULE_NAME)

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm -f modules.order Module.symvers Module.markers

clean_modules:
	dmesg -C
	rm -f dmesg.log
	- sudo rmmod $(MODULE_NAME).ko
	sudo insmod $(MODULE_NAME).ko
	dmesg >> dmesg.log

.PHNOY:
	modules modules_install clean



endif