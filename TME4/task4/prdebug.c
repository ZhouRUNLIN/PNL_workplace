#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kernel_stat.h>

MODULE_DESCRIPTION("A pr_debug kernel module");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");

static struct timer_list prdebug_timer;
static unsigned long irqs_sum_last;

static void prdebug_timeout(struct timer_list *timer)
{
	unsigned long irqs_now = kstat_cpu(0).irqs_sum;

	pr_debug("nr irqs %lu\n", irqs_now - irqs_sum_last);
	irqs_sum_last = irqs_now;

	pr_debug("reactivating timer\n");
	mod_timer(timer, jiffies + HZ);
}

static int __init prdebug_init(void)
{
	irqs_sum_last = kstat_cpu(0).irqs_sum;

	timer_setup(&prdebug_timer, prdebug_timeout, 0);
	mod_timer(&prdebug_timer, jiffies + HZ);

	pr_info("prdebug module loaded\n");
	return 0;
}
module_init(prdebug_init);

static void __exit prdebug_exit(void)
{
	del_timer_sync(&prdebug_timer);
	pr_info("prdebug module unloaded\n");
}
module_exit(prdebug_exit);
