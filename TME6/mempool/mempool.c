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
    mutex_lock(&task_monitor->lock);
    list_add(&sample->list, &task_monitor->head);
    task_monitor->count++;
    mutex_unlock(&task_monitor->lock);
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
    mutex_lock(&task_monitor->lock);
    list_for_each_entry_reverse(it, &task_monitor->head, list) {
        current_size -= scnprintf(buf + (PAGE_SIZE - current_size), current_size, "pid %d usr %llu sys %llu\n", target, 
        it->utime, it->stime);
    }
    mutex_unlock(&task_monitor->lock);
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

unsigned long my_count_objects(struct shrinker *shrinker, struct shrink_control *sc)
{
    return task_monitor->count;
}

unsigned long my_scan_objects(struct shrinker *shrinker, struct shrink_control *sc)
{
    struct task_sample *it;
    unsigned long nmsample = task_monitor->count;
    mutex_lock(&task_monitor->lock);
    while(task_monitor->count * sizeof(struct task_sample) > PAGE_SIZE) {
        it = list_last_entry(&task_monitor->head, struct task_sample, list);
        list_del(&it->list);
        task_monitor->count--;
        // mempool_free(it, sample_mempool);
        kmem_cache_free(cache, it);
    }
    mutex_unlock(&task_monitor->lock);
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
    put_pid(task_monitor->p);
    put_task_struct(task_monitor->task);
    kfree(task_monitor);
    kmem_cache_destroy(cache);
    mempool_destroy(mempool);
    unregister_shrinker(&shrinker);
}
module_exit(hello_exit);