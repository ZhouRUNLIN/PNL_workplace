#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/list_bl.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/namei.h>

MODULE_DESCRIPTION("weasel module");
MODULE_AUTHOR("CHEN Siyuan, SESI");
MODULE_LICENSE("GPL");

extern unsigned int d_hash_shift;
static unsigned int hash_length;
struct proc_dir_entry *proc_entry;

ssize_t weasel_whoami(struct file *file, char *user_buf, size_t count,  loff_t *offset)
{
    char buf[256] = "I'm a weasel!\n";
    return simple_read_from_buffer(user_buf, count, offset, buf, strlen(buf));
}



int dcache_show(struct seq_file *m, void *v)
{
    struct dentry *dentry;
    struct hlist_bl_node *pos;
    struct path path;
    char buf[256];
    for (int i = 0; i < hash_length; i++) {
        hlist_bl_for_each_entry(dentry, pos, dentry_hashtable+i, d_hash) {
            char * name = dentry_path_raw(dentry, buf, sizeof(buf));
            int error = kern_path(name, LOOKUP_FOLLOW, &path);
            if (! error) {
                seq_printf(m, "%s\n", name);
                path_put(&path);
            } 
        }
    }
    return 0;
}

int dcache_open(struct inode *inode, struct file *file)
{
    return single_open(file, dcache_show, inode->i_private);
}

int pwd_show(struct seq_file *m, void *v)
{
    struct dentry *dentry;
    struct hlist_bl_node *pos;
    struct path path;
    char buf[256];
    for (int i = 0; i < hash_length; i++) {
        hlist_bl_for_each_entry(dentry, pos, dentry_hashtable+i, d_hash) {
            char * name = dentry_path_raw(dentry, buf, sizeof(buf));
            int error = kern_path(name, LOOKUP_FOLLOW, &path);
            if (! error) {
                path_put(&path);
            } else {
                seq_printf(m, "%s\n", name);
            }
        }
    }
    return 0;
}

int pwd_open(struct inode *inode, struct file *file)
{
    return single_open(file, pwd_show, inode->i_private);
}

struct proc_ops weasel_whoami_fops = {
    .proc_read = weasel_whoami,
};

struct proc_ops weasel_dcache_fops = {
    .proc_open = dcache_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};

struct proc_ops weasel_pwd_fops = {
    .proc_open = pwd_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};

static int __init weasel_init(void) {
    hash_length = 1 << d_hash_shift;
    struct dentry *dentry;
    struct hlist_bl_node *pos;

    proc_entry = proc_mkdir("weasel", NULL);
    if (!proc_entry) {
        return -ENOMEM;
    }

    if(proc_create("whoami", 0, proc_entry, &weasel_whoami_fops) == NULL) {
        return -ENOMEM;
    }
    if(proc_create("dcache", 0, proc_entry, &weasel_dcache_fops) == NULL) {
        return -ENOMEM;
    }
    if(proc_create("pwd", 0, proc_entry, &weasel_pwd_fops) == NULL) {
        return -ENOMEM;
    }

    //longest hash table in the kernel
    unsigned int max_size = 0;
    unsigned int total_size = 0;
    for (int i = 0; i < hash_length; i++) {
        int size = 0;
        hlist_bl_for_each_entry(dentry, pos, dentry_hashtable+i, d_hash) {
            size++;
        }
        max_size = max_size < size ? size : max_size;
        total_size += size;
    }
    printk(KERN_INFO "max size: %d\n", max_size);
    printk(KERN_INFO "total size: %d\n", total_size);
    printk(KERN_INFO "Weasel module loaded\n");
    printk(KERN_INFO "dentry cache hash table address: %p\n", dentry_hashtable);
    printk(KERN_INFO "dentry cache hash table size: %d\n", hash_length);

    return 0;  // Non-zero return means that the module couldn't be loaded.
}

static void __exit weasel_exit(void) {
    printk(KERN_INFO "Weasel module unloaded\n");
    remove_proc_entry("whoami", proc_entry);
    remove_proc_entry("dcache", proc_entry);
    remove_proc_entry("pwd", proc_entry);
    remove_proc_entry("weasel", NULL);
}

module_init(weasel_init);
module_exit(weasel_exit);