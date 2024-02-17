#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

MODULE_DESCRIPTION("A hanging kernel module");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");

static struct task_struct *hanging_thread;

int my_hanging_fn(void *data)
{
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(60*HZ);
	hanging_thread=NULL;
	pr_warn("done waiting\n");

	return 0;
}

static int __init hanging_init(void)
{
	hanging_thread = kthread_run(my_hanging_fn, NULL, "my_hanging_fn");
	pr_warn("Hanging module loaded\n");

	return 0;
}
module_init(hanging_init);

static void __exit hanging_exit(void)
{
	if (hanging_thread)
		kthread_stop(hanging_thread);
	pr_warn("Hanging module unloaded\n");
}
module_exit(hanging_exit);
