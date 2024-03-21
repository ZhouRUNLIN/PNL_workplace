#ifndef TASKMONITOR_H
#define TASKMONITOR_H

#define TM_GET     _IOR('N', 0, int)
#define TM_STOP    _IOR('N', 1, int)
#define TM_START   _IOR('N', 2, int)
#define TM_PID     _IOW('N', 3, pid_t)

struct task_sample {
    unsigned long long utime;
    unsigned long long stime;
};
static struct task_sample sample;
#endif