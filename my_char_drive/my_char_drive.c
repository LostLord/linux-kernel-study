#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>

#define MAX_NUM 1024
#define MAJOR_NUM 260 // 主设备号，未被使用

const int buffer_size = MAX_NUM;
int major = MAJOR_NUM;
char my_drive_name[] = "my_char_drive";

struct my_device {
    struct cdev devm; // 字符设备
    struct semaphore sem; // 互斥信号量
    wait_queue_head_t outq; // 等待队列，实现阻塞操作
    char buffer[MAX_NUM]; // 字符缓冲区
    loff_t write_ptr;
} global_var;

static struct class *my_class;

static ssize_t my_char_drive_read(struct file *, char *, size_t , loff_t *);
static ssize_t my_char_drive_write(struct file *, const char *,size_t , loff_t *);

// 设备的基本入口点结构变量
struct file_operations my_char_drive_fops = {
    .read = my_char_drive_read,
    .write = my_char_drive_write,
};

static ssize_t my_char_drive_read(struct file *filp, char *buf, size_t len, loff_t *off) {
    loff_t f_pos, read_ptr;
    size_t max_len, left_len, head_len;

    // 不可读时，阻塞读进程
    if (wait_event_interruptible(global_var.outq, filp->f_pos < global_var.write_ptr)) {
        return -ERESTARTSYS;
    }

    if (down_interruptible(&global_var.sem)) {
        return -ERESTARTSYS;
    }

    f_pos = filp->f_pos;
    
    if(global_var.write_ptr - f_pos > MAX_NUM) {
        return -EFAULT;
    }

    // 最多可用长度
    left_len = global_var.write_ptr - f_pos;
    max_len = len < left_len ? len : left_len;

    // 读指针对应的下标位置
    read_ptr = f_pos % MAX_NUM;

    left_len = MAX_NUM - read_ptr;
    left_len = max_len < left_len ? max_len : left_len;
    head_len = max_len - left_len;

    if (copy_to_user(buf, &global_var.buffer[read_ptr], left_len)) {
        printk(KERN_ALERT "copy failed\n");
        // 递增信号量的值,并唤醒所有正在等待信号量转为可用状态的进程
        up(&global_var.sem);
        return -EFAULT;
    }
    f_pos += left_len;

    if (head_len > 0) {
        if (copy_to_user(buf + left_len, &global_var.buffer[0], head_len)) {
            printk(KERN_ALERT "copy failed\n");
            // 递增信号量的值,并唤醒所有正在等待信号量转为可用状态的进程
            up(&global_var.sem);
            return -EFAULT;
        }
        f_pos += head_len;
    }
    *off = f_pos;
    up(&global_var.sem);
    return len;
}

static ssize_t my_char_drive_write(struct file *filp, const char *buf, size_t len, loff_t *off) {
    loff_t buff_pos; // 缓冲区对应的下标
    size_t left_len, head_len; // 尾部剩余长度，循环头部剩余长度
    size_t maxLen; // 最多可用长度

    if (down_interruptible(&global_var.sem)) {
        return -ERESTARTSYS;
    }

    printk("into the write function\n");

    maxLen = len < MAX_NUM ? len : MAX_NUM;
    buff_pos = global_var.write_ptr % MAX_NUM;
    left_len = MAX_NUM - buff_pos;
    left_len = maxLen < left_len ? maxLen : left_len;
    head_len = maxLen - left_len;

    if (copy_from_user(&global_var.buffer[buff_pos], buf, left_len)) {
        printk("copy from error");
        up(&global_var.sem);
        return -EFAULT;
    }
    global_var.write_ptr += left_len;

    if (head_len > 0) {
        if (copy_from_user(&global_var.buffer[0], buf + left_len, head_len)) {
            printk("copy from error");
            up(&global_var.sem);
            return -EFAULT;
        }
        global_var.write_ptr += head_len;
    }
    // 可以唤醒读进程
    wake_up_interruptible(&global_var.outq);
    up(&global_var.sem);
    return maxLen;
}

// 内核模块初始化
static int my_char_drive_init(void) {
    int result = 0;
    int error = 0;

    // 用于在驱动程序中定义设备编号，高12位为主设备号，低20位为次设备号
    // 通过MAJOR和MINOR来获得主设备号和次设备号
    dev_t dev = MKDEV(major, 0);
    if (major) {
        // 静态申请设备编号
        result = register_chrdev_region(dev, 1, my_drive_name);
    } else {
        // 动态分配设备编号
        result = alloc_chrdev_region(&dev, 0, 1, my_drive_name);
        major = MAJOR(dev);
    }

    if (result) {
		printk(KERN_INFO "[my_char_device]init failure: register_chrdev\n");
        return result;
	}

    // 注册字符设备驱动，设备号和file_operations进行绑定
    cdev_init(&global_var.devm, &my_char_drive_fops);
    global_var.devm.owner = THIS_MODULE;
    error = cdev_add(&global_var.devm, dev, 1);

    if (error) {
        printk(KERN_INFO "Error %d adding %s device", error, my_drive_name);
    } else {
        sema_init(&global_var.sem, 1); // 初始化信号量
        init_waitqueue_head(&global_var.outq); // 初始化等待队列
        global_var.write_ptr = 0; // 写指针
        printk(KERN_INFO "globalvar register success\n");
    }

    // 在驱动初始化的代码里调用class_create为该设备创建一个class，再为每个设备调用device_create创建对应的设备。
    // 省去了利用mknod命令手动创建设备节点
    my_class = class_create(THIS_MODULE, "chardev0");
    device_create(my_class, NULL, dev, NULL, "chardev0");

    return 0;
}

static void my_char_drive_exit(void) {
    device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&global_var.devm);
    unregister_chrdev_region(MKDEV(major, 0), 1); // 注销设备
    printk("[my_char_drive]unregister success\n");
}

module_init(my_char_drive_init);
module_exit(my_char_drive_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LostLord");
MODULE_DESCRIPTION("Virtual char device.");