#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
MODULE_DESCRIPTION("Module \"hello world\"");
MODULE_AUTHOR("Julien Sopena, LIP6");
MODULE_LICENSE("GPL");

static int __init hello_init(void)
{
    struct module *mod;
    mod = find_module("hide_module");

    pr_info("Hello, world\n");
	return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
}
module_exit(hello_exit);