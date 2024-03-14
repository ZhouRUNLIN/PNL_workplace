#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include "taskmonitor.h"

MODULE_DESCRIPTION("Monitorsysfs module");
MODULE_AUTHOR("CHEN Siyuan, SESI");
MODULE_LICENSE("GPL");

static int major;

struct task_monitor {
    struct pid *p;
    struct task_struct *task;
};

struct task_monitor* task_monitor;
bool is_stopped = false;

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

bool get_sample(struct task_monitor *task_monitor, struct task_sample *sample) {

        pr_info("test");
        sample->utime = task_monitor->task->utime;
        sample->stime = task_monitor->task->stime;
        pr_info("%lld\n",task_monitor->task->utime);
        pr_info("%lld\n",task_monitor->task->stime);
        return true;
}

int monitor_fn(void *arg)
{
    task_monitor->task = get_pid_task(task_monitor->p, PIDTYPE_PID);  
    struct task_sample t_sample;
    
    while(!kthread_should_stop()){
        if(pid_alive(task_monitor->task) == 0) {
            get_sample(task_monitor, &t_sample);
        } else {
            printk("pid %d usr %llu sys %llu\n", target, task_monitor->task->utime, task_monitor->task->stime);
        }
        ssleep(3);
    }
    
    return 0;
}

void stop_thread(void) {
    if (thread && !is_stopped) {
        kthread_stop(thread);
        wake_up_process(thread);
    }
    is_stopped = true;
}

static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "pid %d usr %lld sys %lld\n", task_monitor->p->numbers[0].nr, task_monitor->task->utime, task_monitor->task->stime);
}

static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
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

static long taskmonitor_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case TM_GET:
            struct task_sample t_sample;
            get_sample(task_monitor, &t_sample);
            pr_info("pid %d usr %llu sys %llu\n", target, task_monitor->task->utime, task_monitor->task->stime);
            break;
        case TM_START:
            pr_info("starting monitor thread\n");
            thread = kthread_run(monitor_fn, NULL, "my_thread");
            is_stopped = false;
            break;
        case TM_STOP:
            pr_info("halting monitor thread\n");
            stop_thread();
            break;
        case TM_PID:
            pr_info("monitoring pid: %d\n", arg);
            if (arg > 0) {
                target = arg;
                if(monitor_pid(target) == -1) {
                    return -1;
                }
                stop_thread();
                thread = kthread_run(monitor_fn, NULL, "my_thread");
            }
            else {
                pr_info("invalid pid\n");

                return -EINVAL;
            }
            break;
        default:
            pr_info("invalid command\n");
            return -EINVAL;
    }
    return 0;
}

struct file_operations taskmonitor_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = taskmonitor_ioctl,
};  // file operations

static int __init hello_init(void) {
    if(monitor_pid(target) == -1) {
        return -1;
    }
    major = register_chrdev(0, "taskmonitor", &taskmonitor_fops);
    if (major < 0) {
        pr_info("failed to register major number\n");
        return major;
    }

    pr_info(
        "Major number = %d\n"
        "Create device with: sudo mknod /dev/taskmonitor c %d 0\n",
        major, major);
    if(sysfs_create_file(kernel_kobj, &(kernel_kobj_attribute.attr))) {
        return -1;
    }

    thread = kthread_run(monitor_fn, NULL, "my_thread");
    return 0;
}
module_init(hello_init);

static void __exit hello_exit(void) {
    stop_thread();
    kobject_put(kernel_kobj);
    sysfs_remove_file(kernel_kobj, &(kernel_kobj_attribute.attr));
    unregister_chrdev(major, "taskmonitor");
    put_pid(task_monitor->p);
    put_task_struct(task_monitor->task);
    kfree(task_monitor);
    pr_info("taskmonitor module unloaded\n");
}
module_exit(hello_exit);