#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cpumask.h>
#include <linux/list.h>
#include <asm-generic/local.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>

static int replace = 1;
static char *modname = NULL;

void force_replace_exit_module_function(void) {
    printk("target module exit success");
}

static int force_cleanup_module(char *del_mod_name) {
    struct module *mod = NULL, *relate = NULL;
    int cpu = 0;
    void *origin_exit_addr = NULL;

    struct module *list_mod = NULL;
    /*  遍历模块列表, 查找 del_mod_name 模块  */
    list_for_each_entry(list_mod, THIS_MODULE->list.prev, list) {
        if (strcmp(list_mod->name, del_mod_name) == 0) {
            mod = list_mod;
        }
    }
    /*  如果未找到 del_mod_name 则直接退出  */
    if(mod == NULL) {
        printk("[%s] module %s not found\n", THIS_MODULE->name, del_mod_name);
        return -1;
    }

    // 如果有其他驱动依赖于当前驱动, 则不能强制卸载, 立刻退出
    // 如果有其他模块依赖于 del_mod
    if (!list_empty(&mod->source_list)) {
        list_for_each_entry(relate, &mod->source_list, source_list) {
            printk("[relate]:%s\n", relate->name);
        }
    } else {
        printk("No modules depond on %s...\n", del_mod_name);
    }

    // 清除驱动的状态和引用计数
    // 修正驱动的状态为LIVE
    mod->state = MODULE_STATE_LIVE;

    //  清除驱动的引用计数
    for_each_possible_cpu(cpu) {
        // local_set((local_t*)per_cpu_ptr(&(mod->refcnt), cpu), 0);
        // local_set(__module_ref_addr(mod, cpu), 0);
        // per_cpu_ptr(mod->refptr, cpu)->decs;
        // module_put(mod);
    }
    atomic_set(&mod->refcnt, 1);

    if (replace != 0) {
        printk("replace exit function\n");
        origin_exit_addr = mod->exit;
        mod->exit = force_replace_exit_module_function;
    }

    printk("[after] name:%s, state:%d, refcnt:%u\n", mod->name, mod->state, module_refcount(mod));
    return 0;
}

static int __init force_rmmod_init(void)
{
    return force_cleanup_module(modname);
}


static void __exit force_rmmod_exit(void)
{
    printk("=======name : %s, state : %d EXIT=======\n", THIS_MODULE->name, THIS_MODULE->state);
}

module_init(force_rmmod_init);
module_exit(force_rmmod_exit);
module_param(replace, int, 0644);
module_param(modname, charp, 0644);

MODULE_LICENSE("GPL");