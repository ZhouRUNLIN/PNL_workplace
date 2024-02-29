#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_DESCRIPTION("taskmonitor module");
MODULE_AUTHOR("CHEN Siyuan, SESI");
MODULE_LICENSE("GPL");

struct task_monitor {
    struct pid *p;
    struct task_struct *task;
};

struct task_monitor* task_monitor;

static struct task_struct *thread;



static pid_t target = 1;
module_param(target, int, 0660);

int monitor_pid(pid_t pid) 
{
    task_monitor = kmalloc(sizeof(struct task_monitor),GFP_KERNEL);
    if(task_monitor == NULL) {
        pr_info("kmalloc failed\n");
        return -1;
    }
    struct pid *p = find_get_pid(pid);
    if(p == NULL) {
        pr_info("pid not found\n");
        return -1;
    }
    task_monitor->p = p;
    return 0;
}

int monitor_fn(void *arg)
{
    task_monitor->task = get_pid_task(task_monitor->p, PIDTYPE_PID); 
    for (size_t i = 0; i < 30; i++)
    {
        if(pid_alive(task_monitor->task) == 0) {
            printk("pid %d usr %lld sys %lld\n", task_monitor->p->numbers[0].nr, task_monitor->task->utime, task_monitor->task->stime);
        } else {
            printk("pid %d usr %lld sys %lld\n", target, task_monitor->task->utime, task_monitor->task->stime);
        }
        msleep(1000);
    }
    return 0;
}

static int __init hello_init(void) {
    pr_info("monitoring pid: %d\n", target);
    if(monitor_pid(target) == -1) {
        return -1;
    }

    thread = kthread_run(monitor_fn, NULL, "my_thread");
    if (IS_ERR(thread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(thread);
    }
    // if (send_sig(SIGKILL, task_monitor->task, 0) < 0) {
    //     printk(KERN_INFO "Failed to send SIGKILL to PID: %d\n", target);
    //     return -EPERM; 
    // }
    pr_info("%d\n", task_monitor->p->numbers[0].nr);
    return 0;
}
module_init(hello_init);

static void __exit hello_exit(void) {
    if (thread) {
        kthread_stop(thread);
        wake_up_process(thread);
    }
    printk(KERN_INFO "Taskmonitor module unloaded\n");
    put_pid(task_monitor->p);
    put_task_struct(task_monitor->task);
    kfree(task_monitor);
}
module_exit(hello_exit);