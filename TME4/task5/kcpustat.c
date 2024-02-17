#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/kernel_stat.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("A cpustat kernel module");
MODULE_AUTHOR("Maxime Lorrillere <maxime.lorrillere@lip6.fr>");
MODULE_LICENSE("GPL");

static unsigned interval = 10; /* interval between reports in seconds */
static unsigned frequency = HZ; /* samples frequency */

static struct task_struct *kcpustat_thread;

struct my_kcpustat {
	struct list_head list;
	struct kernel_cpustat stat;
};

LIST_HEAD(my_head);

/*
 * Save per-CPU-usage stats
 */
int save_stats(void)
{
	int cpu, i;
	struct my_kcpustat *s = kzalloc(sizeof(*s), GFP_KERNEL);

	if (!s)
		return -ENOMEM;

	for (i = 0; i < NR_STATS; i++) {
		for_each_possible_cpu(cpu) {
			struct kernel_cpustat k = kcpustat_cpu(cpu);
			s->stat.cpustat[i] += k.cpustat[i];
		}
	}

	list_add_tail(&s->list, &my_head);
	return 0;
}

/*
 * Show saved stats
 */
void print_stats(void)
{
	struct my_kcpustat *k, *first, *prev;
	struct kernel_cpustat sum = {.cpustat = {0} };
	int nr = 0, i;

	first = list_first_entry(&my_head, struct my_kcpustat, list);

	/*
	 * kernel_cpustats are cumulative, make them noncumulative
	 */
	list_for_each_entry_reverse(k, &my_head, list) {
		if (k != first) {
			prev = list_entry(k->list.prev,
					struct my_kcpustat, list);
			for (i = 0; i < NR_STATS; i++)
				k->stat.cpustat[i] -= prev->stat.cpustat[i];
		}
	}
	list_del(&first->list);

	/*
	 * add samples to display the total CPU usage during time interval
	 */
	list_for_each_entry(k, &my_head, list) {
		for (i = 0; i < NR_STATS; i++)
			sum.cpustat[i] += k->stat.cpustat[i];
		list_del(&k->list);
		nr++;
	}

	pr_warn("usr %llu sys %llu idle %llu iowait %llu irq %llu softirq %llu\n",
			sum.cpustat[CPUTIME_USER],
			sum.cpustat[CPUTIME_SYSTEM],
			sum.cpustat[CPUTIME_IDLE],
			sum.cpustat[CPUTIME_IOWAIT],
			sum.cpustat[CPUTIME_IRQ],
			sum.cpustat[CPUTIME_SOFTIRQ]);

	list_del(&first->list);
	list_del_init(&my_head);
}

int my_kcpustat_fn(void *data)
{
	int n = 0, err;

	while (!kthread_should_stop()) {
		err = save_stats();
		if (err)
			return err;

		if (++n % (interval * (HZ / frequency)) == 0)
			print_stats();

		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(frequency);
	}
	return 0;
}

static int __init kcpustat_init(void)
{
	kcpustat_thread = kthread_run(my_kcpustat_fn, NULL, "my_kcpustat_fn");
	pr_warn("Greedy module loaded\n");
	return 0;
}
module_init(kcpustat_init);

static void __exit kcpustat_exit(void)
{
	if (kcpustat_thread)
		kthread_stop(kcpustat_thread);
	pr_warn("Greedy module unloaded\n");
}
module_exit(kcpustat_exit);
