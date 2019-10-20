#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <asm/processor.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FinnTenzor");
MODULE_DESCRIPTION("My system call.");

unsigned long** p_sys_call_table = NULL;
unsigned long* old_sys_call = NULL;
int replace_sys_call_no = 333; // 定值，指向自己新加的系统调用号

// 关闭写保护
void close_write_protection(void) {
	write_cr0(read_cr0() & (~0x10000));
}

// 启用写保护
void open_write_protection(void) {
	write_cr0(read_cr0() | 0x10000);
}

// 初始化系统调用表地址
void init_sys_call_table(void) {
	if (p_sys_call_table == NULL) {
		p_sys_call_table = (unsigned long**)kallsyms_lookup_name("sys_call_table");
	}
}

// 设置系统调用
void set_system_call(int no, unsigned long* func) {
	if (p_sys_call_table == NULL) {
		printk("Error at set system call, not init!\n");
		return;
	}
	// close_write_protection();
	p_sys_call_table[no] = func;
	// open_write_protection();
}

// 新的系统调用
asmlinkage long chang_sys_call(char* to1, char* to2) {
    char s[30] = {"abvcdffas"};
    copy_to_user(to1, s, sizeof(s));

	printk("has changed");

	return 0;
// chang_sys_call_error:
// 	return ret;
}

// 修改系统调用
void modify_syscall(void) {
	if (old_sys_call == NULL) {
		old_sys_call = p_sys_call_table[replace_sys_call_no];
		set_system_call(replace_sys_call_no, (unsigned long*)&chang_sys_call);
	} // else 重入，不能再次替换，否则会导致系统调用无法恢复
}

// 恢复系统调用
void restore_syscall(void) {
	if (old_sys_call == NULL) {
		printk("[chang_sys_call]Restore syscall failed!\n");
		return;
	}
	set_system_call(replace_sys_call_no, old_sys_call);
	old_sys_call = NULL; // 保证可重入性
}

// 模块初始化
static int __init chang_sys_call_init(void) {
	init_sys_call_table();
	modify_syscall();
	printk(KERN_INFO "[chang_sys_call]init\n");
	return 0;
}

// 模块卸载
static void __exit chang_sys_call_exit(void) {
	restore_syscall();
	printk(KERN_INFO "[chang_sys_call]exit!\n");
}

// 注册模块
module_init(chang_sys_call_init);
module_exit(chang_sys_call_exit);
