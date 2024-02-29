#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>

MODULE_DESCRIPTION("Module \"hellosysfs\"");
MODULE_AUTHOR("Siyuan Chen, SESI");
MODULE_LICENSE("GPL");

static char target[20] = "sysfs";

static ssize_t hello_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    return sprintf(buf, "Hello %s!\n",target);
}

static ssize_t hello_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){

    if (sscanf(buf, "%s", target) != 1)
        return -EINVAL;
    
    return count;
}

static struct kobj_attribute kernel_kobj_attribute = __ATTR_RW(hello);

static int __init my_init(void) {

    if (sysfs_create_file(kernel_kobj, &(kernel_kobj_attribute.attr)))
        return -1;
    pr_info("Module \"hellosysfs\" loaded\n");
    return 0;
}
module_init(my_init);

static void __exit my_exit(void){
    sysfs_remove_file(kernel_kobj, &(kernel_kobj_attribute.attr));
    pr_info("Module \"hellosysfs\" unloaded\n");
}
module_exit(my_exit);