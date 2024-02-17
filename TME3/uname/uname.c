#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>


MODULE_DESCRIPTION("Module \"uname\"");
MODULE_AUTHOR("Siyuan Chen, SESI");
MODULE_LICENSE("GPL");

static char original_release[__NEW_UTS_LEN + 1];
static char original_sysname[__NEW_UTS_LEN + 1];
//sizeof(init_uts_ns.name.release)

static int __init uname_init(void)
{
    strcpy(original_release, init_uts_ns.name.release);
    strcpy(original_sysname, init_uts_ns.name.sysname);
    pr_info("%s\n",original_release);
    pr_info("%s\n",original_sysname);

    strcpy(init_uts_ns.name.release, "my custom kernel version");
    strcpy(init_uts_ns.name.sysname, "kernal_for_chen");
    return 0;
}
module_init(uname_init);

static void __exit uname_exit(void)
{
    strcpy(init_uts_ns.name.release, original_release);
    strcpy(init_uts_ns.name.sysname, original_sysname);

    return;
}
module_exit(uname_exit);