#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_DESCRIPTION("Monitorsysfs module");
MODULE_AUTHOR("CHEN Siyuan, SESI");
MODULE_LICENSE("GPL");

struct task_monitor {
    struct pid *p;
    struct task_struct *task;
};

struct task_monitor* task_monitor;
bool is_stopped = false;

static struct task_struct *thread;

struct task_sample{
    u64 utime;
    u64 stime;
};

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

bool get_sample(struct task_monitor *task_monitor, struct task_sample *sample) 
{
    if (pid_alive(task_monitor->task) == 0) {
        sample->utime = task_monitor->task->utime;
        sample->stime = task_monitor->task->stime;
        return true;
    }
    return false;
}

int monitor_fn(void *arg)
{
    task_monitor->task = get_pid_task(task_monitor->p, PIDTYPE_PID); 
    struct task_sample sample;

    while(!kthread_should_stop()){
        if(pid_alive(task_monitor->task) == 0) {
            get_sample(task_monitor, &sample);
        } else {
            printk("pid %d usr %llu sys %llu\n", target, sample.utime, sample.stime);
        }
        ssleep(3);
    }
    
    return 0;
}

void stop_thread(void) 
{
    if (thread && !is_stopped) {
        kthread_stop(thread);
        wake_up_process(thread);
    }
}

static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{
    return sprintf(buf, "pid %d usr %lld sys %lld\n", task_monitor->p->numbers[0].nr, task_monitor->task->utime, task_monitor->task->stime);
}

static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) 
{
    if (strcmp(buf, "stop") == 0) {
        pr_info("halting monitor thread\n");
        stop_thread();
        is_stopped = true;
    }
    else if (strcmp(buf, "start") == 0) {
        pr_info("starting monitor thread\n");
        thread = kthread_run(monitor_fn, NULL, "my_thread");
        is_stopped = false;
    } else {
        pr_info("invalid command\n");
    }
    return count;
}

static struct kobj_attribute kernel_kobj_attribute = __ATTR_RW(taskmonitor);

static int __init hello_init(void) 
{
    pr_info("monitoring pid: %d\n", target);
    if(monitor_pid(target) == -1) {
        return -1;
    }

    thread = kthread_run(monitor_fn, NULL, "my_thread");
    if (IS_ERR(thread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(thread);
    }


    if(sysfs_create_file(kernel_kobj, &(kernel_kobj_attribute.attr))) {
        return -1;
    }

    pr_info("%d\n", task_monitor->p->numbers[0].nr);
    return 0;
}
module_init(hello_init);

static void __exit hello_exit(void) 
{
    stop_thread();
    printk(KERN_INFO "Taskmonitor module unloaded\n");
    kobject_put(kernel_kobj);
    sysfs_remove_file(kernel_kobj, &(kernel_kobj_attribute.attr));
    put_pid(task_monitor->p);
    put_task_struct(task_monitor->task);
    kfree(task_monitor);
}
module_exit(hello_exit);