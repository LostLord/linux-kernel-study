#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>


static int __init test_modules_init(void)
{
    printk("test");
    return 0;
}

static void __exit test_modules_exit(void)
{
    printk("=======name : %s, state : %d EXIT=======\n", THIS_MODULE->name, THIS_MODULE->state);
}

module_init(test_modules_init);
module_exit(test_modules_exit);

MODULE_LICENSE("GPL");