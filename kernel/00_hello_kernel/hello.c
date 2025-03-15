/*
 * hello.c - The simplest kernel module.
 */

#include <linux/init.h>   /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

static int hello_init(void)
{
    pr_info("Hello world generic!\n");
    /* A non 0 return means init_module failed; module can't be loaded. */
    return 0;
}

static void hello_exit(void)
{
    pr_info("Goodbye world generic!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
