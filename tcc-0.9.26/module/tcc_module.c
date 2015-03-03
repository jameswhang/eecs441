#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <asm/uaccess.h>
#include <linux/cdev.h>

#include "../libtcc.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Northwestern University");
MODULE_DESCRIPTION("TCC_Module");

static int Major;
dev_t tcc_dev;

static char * user_program[];
static int __init tcc_module_init(void)
{
    printk(KERN_INFO "TCC Module Inited\n");

    TCCState *s;
    int (*func)(int);

    printk("Starting tcc_new\n");

    s = tcc_new();
    if (!s) {
        printk("Could not create tcc state\n");
	return 0;
    }

    printk("Starting tcc_set_output_type\n");
    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    printk("Starting tcc_compile_string\n");
    if (tcc_compile_string(s, user_program) == -1) {
      printk("Cannot compile program!\n");
      return 0;
    }
    printk("Starting tcc_relocate\n");

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
      printk("Cannot relocate program\n");
      return 0;
    }

    printk("Starting tcc_get_symbol\n");
    /* get entry symbol */
    func = tcc_get_symbol(s, "main");
    if (!func) {
      printk("Cannot find main...\n");
      return 0;
    }

    printk("Calling the code!\n");

    /* run the code */
    void * result = func(10);

    printk("Result is :%d\n", result);

    printk("Deleting state\n");

    tcc_delete(s);
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit tcc_module_deinit(void)
{
    printk(KERN_INFO "TCC Module Deinited\n");
}

void * tcc_kmalloc(size_t n)
{
  return __vmalloc(n, GFP_ATOMIC, PAGE_KERNEL_EXEC);
}

void * tcc_krealloc(void *p, size_t n)
{
  return krealloc(p, n, GFP_ATOMIC);
}

module_init(tcc_module_init);
module_exit(tcc_module_deinit);
