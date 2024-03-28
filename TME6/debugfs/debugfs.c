#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/shrinker.h>
#include <linux/mempool.h>
#include <linux/slab.h>
#include <linux/debugfs.h>

MODULE_DESCRIPTION("mempool module");
MODULE_AUTHOR("CHEN Siyuan, SESI");
MODULE_LICENSE("GPL");

// static mempool_t *sample_mempool;

struct task_monitor {
    struct pid *p;
    struct task_struct *task;
    int count;
    struct kref refcount;
    struct list_head head;
};

struct task_monitor* task_monitor;
bool is_stopped = false;

static struct task_struct *thread;

static struct kmem_cache *cache;

static mempool_t *mempool;

struct task_sample{
    u64 utime;
    u64 stime;
    struct list_head list;

};

/* 实现 open 方法来打开 seq_file */

/* 设置 file_operations 结构 */

bool get_sample(struct task_monitor *task_monitor, struct task_sample *sample) 
{
    if (pid_alive(task_monitor->task) == 1) {
        sample->utime = task_monitor->task->utime + task_monitor->count;
        sample->stime = task_monitor->task->stime + task_monitor->count;
        return true;
    }
    return false;
}

int save_sample(void)
{
    struct task_sample *sample = kmem_cache_alloc(cache, GFP_KERNEL);
    if (sample == NULL) {
        return -1;
    }
    get_sample(task_monitor, sample);
    while(kref_get_unless_zero(&task_monitor->refcount) == 0);
    list_add(&sample->list, &task_monitor->head);
    task_monitor->count++;
    kref_put(&task_monitor->refcount, NULL);
    return 0;
}

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
    kref_init(&task_monitor->refcount);
    INIT_LIST_HEAD(&task_monitor->head);
    task_monitor->count = 0;
    return 0;
}

int monitor_fn(void *arg)
{
    task_monitor->task = get_pid_task(task_monitor->p, PIDTYPE_PID); 
    struct task_sample sample;

    while(!kthread_should_stop()){
        if(pid_alive(task_monitor->task) == 1) {
            save_sample();
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
        is_stopped = true;
    }
}

static ssize_t taskmonitor_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{
    ssize_t current_size = PAGE_SIZE;
    struct task_sample *it;
    if (!kref_get_unless_zero(&task_monitor->refcount))
        stop_thread();
    else{
        list_for_each_entry_reverse(it, &task_monitor->head, list) {
            current_size -= scnprintf(buf + (PAGE_SIZE - current_size), current_size, "pid %d usr %llu sys %llu\n", target, 
            it->utime, it->stime);
        }
        kref_put(&task_monitor->refcount, NULL);
    }
    return PAGE_SIZE - current_size;
}

static ssize_t taskmonitor_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) 
{
    if (strcmp(buf, "stop") == 0) {
        pr_info("halting monitor thread\n");
        stop_thread();
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

static int my_seq_show(struct seq_file *m, void *v) 
{
    struct task_sample *it;

        list_for_each_entry_reverse(it, &task_monitor->head, list) {
            seq_printf(m, "pid %d usr %llu sys %llu\n", target, it->utime, it->stime);
        }
    return 0;
}
unsigned long my_count_objects(struct shrinker *shrinker, struct shrink_control *sc)
{
    return task_monitor->count;
}

unsigned long my_scan_objects(struct shrinker *shrinker, struct shrink_control *sc)
{
    struct task_sample *it;
    unsigned long nmsample = task_monitor->count;
    if (!kref_get_unless_zero(&task_monitor->refcount))
        stop_thread();
    else{
        while(task_monitor->count * sizeof(struct task_sample) > PAGE_SIZE) {
            it = list_last_entry(&task_monitor->head, struct task_sample, list);
            list_del(&it->list);
            task_monitor->count--;
            kmem_cache_free(cache, it);
        }
        kref_put(&task_monitor->refcount, NULL);
    }
    return nmsample - task_monitor->count;
}

struct shrinker shrinker = {
    .count_objects = &my_count_objects,
    .scan_objects = &my_scan_objects,
    .batch = 0,
    .seeks = DEFAULT_SEEKS,
    .flags = 0,
};

static struct kobj_attribute kernel_kobj_attribute = __ATTR_RW(taskmonitor);

static struct dentry *my_debugfs_file;

static const struct seq_operations my_seq_ops = {
    .show  = my_seq_show
};

static int my_open(struct inode *inode, struct file *file) {
    return seq_open(file, &my_seq_ops);
}

static const struct file_operations my_file_ops = {
    .owner   = THIS_MODULE,
    .open    = my_open,           // 自定义 open 方法
    .llseek  = seq_lseek,         // 通用 llseek 方法
    .release = single_release        // 通用 release 方法
};


static int __init hello_init(void) 
{
    pr_info("monitoring pid: %d\n", target);
    if(monitor_pid(target) == -1) {
        return -1;
    }
    cache = kmem_cache_create("my_cache",
                                 sizeof(struct task_sample),
                                 0,
                                 (SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD),
                                 NULL);
    register_shrinker(&shrinker, "my_shrinker");
    mempool = mempool_create_slab_pool(100, cache);
    thread = kthread_run(monitor_fn, NULL, "my_thread");
    if (IS_ERR(thread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(thread);
    }


    if(sysfs_create_file(kernel_kobj, &(kernel_kobj_attribute.attr))) {
        return -1;
    }

    my_debugfs_file = debugfs_create_file("taskmonitor", 0644, NULL, NULL, &my_file_ops);
    if(my_debugfs_file == NULL) {
        pr_info("debugfs_create_file failed\n");
        return -1;
    }

    pr_info("%d\n", task_monitor->p->numbers[0].nr);
    pr_info("sizeof(struct task_monitor) %lu\n", sizeof(struct task_monitor));
    pr_info("ksize(struct task_monitor) %lu\n", ksize(task_monitor));
    return 0;
}
module_init(hello_init);

static void __exit hello_exit(void) 
{
    stop_thread();
    printk(KERN_INFO "Taskmonitor module unloaded\n");
    kobject_put(kernel_kobj);
    sysfs_remove_file(kernel_kobj, &(kernel_kobj_attribute.attr));
    debugfs_remove(my_debugfs_file);
    put_pid(task_monitor->p);
    put_task_struct(task_monitor->task);
    kfree(task_monitor);
    kmem_cache_destroy(cache);
    mempool_destroy(mempool);
    unregister_shrinker(&shrinker);
}
module_exit(hello_exit);