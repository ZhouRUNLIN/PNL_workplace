#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

MODULE_DESCRIPTION("Module \"superblock\"");
MODULE_AUTHOR("Siyuan Chen, SESI");
MODULE_LICENSE("GPL");

static char* param;
module_param(param, charp, 0660);
struct file_system_type *fst;
static ktime_t timestamp = 0;

void show(struct super_block *sb, void *data)
{
    printk("uuid=%pUb type=%s, time = %lld\n", &(sb->s_uuid), sb->s_type->name, timestamp);
}

void acquire_type(void)
{
    fst = get_fs_type(param);
    if(fst)
    {
        iterate_supers_type(fst,&show, NULL);
    } else {
        pr_info("No such file system type\n");
    } 
    timestamp = ktime_get();
}

void release_type(void)
{
    if(fst){
        put_filesystem(fst);    
    }
}

static int __init show_sb_init(void)
{   
    acquire_type();
    return 0;
}
module_init(show_sb_init);
static void __exit hello_exit(void) {
    release_type();
    pr_info("Goodbye! \n");
}
module_exit(hello_exit);

