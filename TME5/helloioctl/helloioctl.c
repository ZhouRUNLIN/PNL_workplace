#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include "request.h"

MODULE_DESCRIPTION("Module \"helloioctl\"");
MODULE_AUTHOR("Siyuan Chen, SESI");
MODULE_LICENSE("GPL");

static int major;


static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    pr_info("ioctl\n");
    if (_IOC_TYPE(cmd) != 'N') {
        pr_info("Invalid type\n");
        return -ENOTTY;
    }
    switch(cmd) {
        case HELLO:
            pr_info("HELLO\n");
            if (copy_to_user((char *)arg, msg, 100))
                return -EFAULT;
            break;
        default:
            pr_info("Invalid command\n");
            return -ENOTTY;
    }

    return 0;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_ioctl,
};  // file operations

static int __init my_init(void) {
    major = register_chrdev(major, "hello", &fops);
    if (major < 0)
        return -1;
    pr_info("Module \"helloioctl\" loaded\n");
    pr_info(
        "Major number = %d\n"
        "Create device with: sudo mknod /dev/hello c %d 0\n",
        major, major);
    return 0;
}
module_init(my_init);

static void __exit my_exit(void){
    unregister_chrdev(major, "hello"); 
    pr_info("Module \"helloioctl\" unloaded\n");
}
module_exit(my_exit);