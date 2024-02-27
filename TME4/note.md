## kgdb
kernel hacking -> Generic kernel Debugging Instruments -> KGDB

kgdb configure by socket tcp 1234

ajouter dans .sh :-nographic

`echo 'g' > /proc/sysrq-trigger`: switch between vm et kgdb

pas utilise make defconfig dans tp2

make nconfig -> Fn+ F8 pour cherche une fonction dans config.

```
(gdb) print init_uts_ns
$2 = {name = {sysname = "Linux", '\000' <repeats 59 times>, nodename = "pnl", '\000' <repeats 61 times>, release = "6.5.7", '\000' <repeats 59 times>, 
    version = "#19 SMP PREEMPT_DYNAMIC Thu Feb 15 16:21:40 CET 2024", '\000' <repeats 12 times>, machine = "x86_64", '\000' <repeats 58 times>, 
    domainname = "(none)", '\000' <repeats 58 times>}, user_ns = 0xffffffff82a51780 <init_user_ns>, ucounts = 0x0 <fixed_percpu_data>, ns = {stashed = {counter = 0}, 
    ops = 0xffffffff82212680 <utsns_operations>, inum = 4026531838, count = {refs = {counter = 4}}}}
(gdb) set init_uts_ns.name.release="string"
$3 = "string", '\000' <repeats 58 times>
(gdb) continue
Continuing.
```


CONFIG_PANIC_ON_OOPS
CONFIG_BOOTPARAM_HUNG_TASK_PANIC


panic avec hung_task, corrrespondre le résultat.
```
(gdb) bt
#0  kgdb_breakpoint () at kernel/debug/debug_core.c:1219
#1  0xffffffff81086a34 in panic (fmt=fmt@entry=0xffffffff826cf613 "hung_task: blocked tasks") at kernel/panic.c:348
#2  0xffffffff8118b790 in check_hung_uninterruptible_tasks (timeout=<optimized out>) at kernel/hung_task.c:226
#3  watchdog (dummy=<optimized out>) at kernel/hung_task.c:379
#4  0xffffffff810b2d51 in kthread (_create=0xffff888003e23e40) at kernel/kthread.c:389
#5  0xffffffff8103ac8f in ret_from_fork (prev=<optimized out>, regs=0xffffc900000abf58, fn=0xffffffff810b2c70 <kthread>, fn_arg=0xffff888003e23e40)
    at arch/x86/kernel/process.c:145
#6  0xffffffff810023db in ret_from_fork_asm () at arch/x86/entry/entry_64.S:304
#7  0x0000000000000000 in ?? ()
```

```
(gdb) monitor ps
48 sleeping system daemon (state [ims]) processes suppressed,
use 'ps A' to see all.
Task Addr               Pid   Parent [*] cpu State Thread             Command
0xffff888003dcab80       20        2  1    0   R  0xffff888003dcb6c0 *khungtaskd

0xffff888003da8000        1        0  0    0   S  0xffff888003da8b40  systemd
0xffff888003dcab80       20        2  1    0   R  0xffff888003dcb6c0 *khungtaskd
0xffff88803fb71d00       73        1  0    0   R  0xffff88803fb72840  systemd-journal
0xffff88803fb70000       84        1  0    0   S  0xffff88803fb70b40  systemd-udevd
0xffff88803fb75700       88        1  0    0   S  0xffff88803fb76240  systemd-network
0xffff88803e608000      116        1  0    0   S  0xffff88803e608b40  acpid
0xffff88803e608e80      117        1  0    0   S  0xffff88803e6099c0  dbus-daemon
0xffff88803e60c880      118        1  0    0   S  0xffff88803e60d3c0  systemd-logind
0xffff88803e60e580      120        1  0    0   S  0xffff88803e60f0c0  login
0xffff88803e60d700      121        1  0    0   S  0xffff88803e60e240  login
0xffff88803fb78000      123        1  0    0   S  0xffff88803fb78b40  systemd
0xffff88803fb79d00      124      123  0    0   S  0xffff88803fb7a840  (sd-pam)
0xffff88803d018e80      130      121  0    0   S  0xffff88803d0199c0  bash
0xffff88803d019d00      131      120  0    0   S  0xffff88803d01a840  bash
0xffff88803d01ab80      142        2  0    0   D  0xffff88803d01b6c0  my_hanging_fn
(gdb) monitor btp
(gdb) monitor lsmod
Module                  Size  modstruct     Used by
hanging                 4096/    4096/       0/    4096  0xffffffffa0002040    0  (Live) 0xffffffffa0000000/0xffffffffa0004000/0x0000000000000000/0xffffffffa0002000 [ ]

```

```
(gdb) monitor ps A
Task Addr               Pid   Parent [*] cpu State Thread             Command
0xffff888003dcab80       20        2  1    0   R  0xffff888003dcb6c0 *khungtaskd

0xffff888003da8000        1        0  0    0   S  0xffff888003da8b40  systemd
0xffff888003da8e80        2        0  0    0   s  0xffff888003da99c0  kthreadd
0xffff888003da9d00        3        2  0    0   i  0xffff888003daa840  rcu_gp
0xffff888003daab80        4        2  0    0   i  0xffff888003dab6c0  rcu_par_gp
0xffff888003daba00        5        2  0    0   i  0xffff888003dac540  slub_flushwq
0xffff888003dac880        6        2  0    0   i  0xffff888003dad3c0  netns
0xffff888003dad700        7        2  0    0   i  0xffff888003dae240  kworker/0:0
0xffff888003dae580        8        2  0    0   i  0xffff888003daf0c0  kworker/0:0H
0xffff888003db8000        9        2  0    0   i  0xffff888003db8b40  kworker/0:1
0xffff888003db8e80       10        2  0    0   i  0xffff888003db99c0  kworker/u2:0
0xffff888003db9d00       11        2  0    0   i  0xffff888003dba840  mm_percpu_wq
0xffff888003dbab80       12        2  0    0   i  0xffff888003dbb6c0  rcu_tasks_kthre
0xffff888003dbba00       13        2  0    0   s  0xffff888003dbc540  ksoftirqd/0
0xffff888003dbc880       14        2  0    0   i  0xffff888003dbd3c0  rcu_preempt
0xffff888003dbd700       15        2  0    0   s  0xffff888003dbe240  migration/0
0xffff888003dbe580       16        2  0    0   s  0xffff888003dbf0c0  cpuhp/0
0xffff888003dc8000       17        2  0    0   s  0xffff888003dc8b40  kdevtmpfs
0xffff888003dc8e80       18        2  0    0   i  0xffff888003dc99c0  inet_frag_wq
0xffff888003dc9d00       19        2  0    0   s  0xffff888003dca840  kauditd
0xffff888003dcab80       20        2  1    0   R  0xffff888003dcb6c0 *khungtaskd
0xffff888003dcba00       21        2  0    0   i  0xffff888003dcc540  kworker/u2:1
0xffff888003dcc880       22        2  0    0   s  0xffff888003dcd3c0  oom_reaper
0xffff888003dcd700       23        2  0    0   i  0xffff888003dce240  writeback
0xffff888003dce580       24        2  0    0   s  0xffff888003dcf0c0  kcompactd0
0xffff888003e90000       25        2  0    0   i  0xffff888003e90b40  kblockd
0xffff888003e90e80       26        2  0    0   i  0xffff888003e919c0  ata_sff
0xffff888003e91d00       27        2  0    0   i  0xffff888003e92840  md
0xffff888003e92b80       28        2  0    0   i  0xffff888003e936c0  md_bitmap
0xffff888003e93a00       29        2  0    0   i  0xffff888003e94540  kworker/u2:2
0xffff888003e94880       30        2  0    0   i  0xffff888003e953c0  kworker/0:1H
0xffff888003e95700       31        2  0    0   i  0xffff888003e96240  rpciod
0xffff888003e96580       32        2  0    0   i  0xffff888003e970c0  xprtiod
0xffff8880048b8000       33        2  0    0   i  0xffff8880048b8b40  cfg80211
0xffff8880048b8e80       34        2  0    0   s  0xffff8880048b99c0  kswapd0
0xffff8880048b9d00       35        2  0    0   i  0xffff8880048ba840  nfsiod
0xffff8880048bab80       36        2  0    0   i  0xffff8880048bb6c0  acpi_thermal_pm
0xffff8880048bba00       37        2  0    0   s  0xffff8880048bc540  scsi_eh_0
0xffff8880048bc880       38        2  0    0   i  0xffff8880048bd3c0  scsi_tmf_0
0xffff8880048bd700       39        2  0    0   s  0xffff8880048be240  scsi_eh_1
0xffff8880048be580       40        2  0    0   i  0xffff8880048bf0c0  scsi_tmf_1
0xffff888004978000       41        2  0    0   i  0xffff888004978b40  kworker/u2:3
0xffff888004978e80       42        2  0    0   i  0xffff8880049799c0  kworker/u2:4
0xffff888004979d00       43        2  0    0   i  0xffff88800497a840  kworker/0:2
0xffff88800497ab80       44        2  0    0   i  0xffff88800497b6c0  mld
0xffff88800497ba00       45        2  0    0   i  0xffff88800497c540  ipv6_addrconf
0xffff88800497e580       49        2  0    0   s  0xffff88800497f0c0  jbd2/sda1-8
0xffff88800497d700       50        2  0    0   i  0xffff88800497e240  ext4-rsv-conver
0xffff88803fb71d00       73        1  0    0   R  0xffff88803fb72840  systemd-journal
0xffff88803fb70000       84        1  0    0   S  0xffff88803fb70b40  systemd-udevd
0xffff88803fb75700       88        1  0    0   S  0xffff88803fb76240  systemd-network
0xffff88803e609d00      111        2  0    0   s  0xffff88803e60a840  jbd2/sdb-8
0xffff88803e60ab80      112        2  0    0   i  0xffff88803e60b6c0  ext4-rsv-conver
0xffff88803e60ba00      115        2  0    0   i  0xffff88803e60c540  kworker/0:3
0xffff88803e608000      116        1  0    0   S  0xffff88803e608b40  acpid
0xffff88803e608e80      117        1  0    0   S  0xffff88803e6099c0  dbus-daemon
0xffff88803e60c880      118        1  0    0   S  0xffff88803e60d3c0  systemd-logind
0xffff88803e60e580      120        1  0    0   S  0xffff88803e60f0c0  login
0xffff88803e60d700      121        1  0    0   S  0xffff88803e60e240  login
0xffff88803fb78000      123        1  0    0   S  0xffff88803fb78b40  systemd
0xffff88803fb79d00      124      123  0    0   S  0xffff88803fb7a840  (sd-pam)
0xffff88803d018e80      130      121  0    0   S  0xffff88803d0199c0  bash
0xffff88803d019d00      131      120  0    0   S  0xffff88803d01a840  bash
0xffff88803d01ab80      142        2  0    0   D  0xffff88803d01b6c0  my_hanging_fn
```

find . -name "*.c" | xargs grep lsmod
```
siyuanchen@de:~/pnl_workplace/linux-6.5.7$ find . -name "*.c" | xargs grep lsmod 
./kernel/debug/kdb/kdb_main.c: * forcing them to have no arguments) and lsmod. These commands do expose
./kernel/debug/kdb/kdb_main.c:	{	.name = "lsmod",
./kernel/debug/kdb/kdb_main.c:		.func = kdb_lsmod,
./kernel/module/kdb.c: * kdb_lsmod - This function implements the 'lsmod' command.  Lists
./kernel/module/kdb.c: *	Mostly taken from userland lsmod.
./kernel/module/kdb.c:int kdb_lsmod(int argc, const char **argv)
./samples/nitro_enclaves/ne_ioctl_sample.c: *	lsmod
./samples/nitro_enclaves/ne_ioctl_sample.c: *	lsmod
./drivers/mfd/omap-usb-tll.c:static unsigned ohci_omap3_fslsmode(enum usbhs_omap_port_mode mode)
./drivers/mfd/omap-usb-tll.c:				reg |= ohci_omap3_fslsmode(pdata->port_mode[i])
```

minitor help
```
(gdb) monitor help
Command         Usage                Description
----------------------------------------------------------
md              <vaddr>             Display Memory Contents, also mdWcN, e.g. md8c1
mdr             <vaddr> <bytes>     Display Raw Memory
mdp             <paddr> <bytes>     Display Physical Memory
mds             <vaddr>             Display Memory Symbolically
mm              <vaddr> <contents>  Modify Memory Contents
go              [<vaddr>]           Continue Execution
rd                                  Display Registers
rm              <reg> <contents>    Modify Registers
ef              <vaddr>             Display exception frame
bt              [<vaddr>]           Stack traceback
btp             <pid>               Display stack for process <pid>
bta             [<state_chars>|A]   Backtrace all processes whose state matches
btc                                 Backtrace current process on each cpu
btt             <vaddr>             Backtrace process given its struct task address
env                                 Show environment variables
set                                 Set environment variables
help                                Display Help Message
?                                   Display Help Message
cpu             <cpunum>            Switch to new cpu
kgdb                                Enter kgdb mode
ps              [<state_chars>|A]   Display active task list
pid             <pidnum>            Switch to another task
reboot                              Reboot the machine immediately
lsmod                               List loaded kernel modules
sr              <key>               Magic SysRq key
dmesg           [lines]             Display syslog buffer
defcmd          name "usage" "help" Define a set of commands, down to endefcmd
kill            <-signal> <pid>     Send a signal to a process
summary                             Summarize the system
per_cpu         <sym> [<bytes>] [<cpu>]
                                    Display per_cpu variables
grephelp                            Display help on | grep
bp              [<vaddr>]           Set/Display breakpoints
bl              [<vaddr>]           Display breakpoints
bc              <bpnum>             Clear Breakpoint
be              <bpnum>             Enable Breakpoint
bd              <bpnum>             Disable Breakpoint
ss                                  Single Step
bph             [<vaddr>]           [datar [length]|dataw [length]]   Set hw brk
dumpcommon                          Common kdb debugging
dumpall                             First line debugging
dumpcpu                             Same as dumpall but only tasks on cpus
ftdump          [skip_#entries] [cpu]
                                    Dump ftrace log; -skip dumps last #entries
```

deuxième l'adresse:l'adresse du code de module.

## task 3:

change code:

TASK_INTERRUPTIBLE

change configuration:

change timeout value to 100(>60) in oops

### task 4

`echo -n 'file prdebug.c line 15 +p' > /sys/kernel/debug/dynamic_debug/control`