KERNEL_DIR:=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)
MODULE_NAME:=change_sys_call

obj-m += $(MODULE_NAME).o
default:build
clean-log:
	dmesg -C
dump-log:
	dmesg >> dmesg.log
build:
	make -C $(KERNEL_DIR) M=$(PWD) modules
clean:clean-test
	make -C $(KERNEL_DIR) M=$(PWD) clean
install:
	insmod $(MODULE_NAME).ko
uninstall:
	- rmmod $(MODULE_NAME).ko
clean-test:
	rm -f test.exe
	rm -f test.out
end:uninstall clean
	rm -f dmesg.log
ins:clean-log install dump-log
test:build-test
	./test.exe > test.out
	date "+%Y-%m-%d %H:%M:%S" >> dmesg.log
	dmesg >> dmesg.log
build-test:
	gcc test.c -o test.exe
