# TME2

## setup

`qemu-system-x86_64 -drive file=lkp-arch.img,format=raw`:

`dd if=/dev/zero of=myHome.img bs=1M count=50`

` mkfs.ext4 myHome.img`


uname -r: `5.11.2-arch1-1`

## general setup

```
make nconfig 
        -> general up
            -> local version - append to kernel release
                    -> nihao

make -j 6: recompilation
        -> lancer le script
                -> affichage de "nihao" après la version

```

## init processus
```
cat test.txt | grep helloworld
[    0.000000] Command line: root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1 init=helloworld
[    0.034826] Kernel command line: root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1 init=helloworld
[    1.987324] Run helloworld as init process
[    1.987956] Kernel panic - not syncing: Requested init helloworld failed (error -2).
[    1.998293] ---[ end Kernel panic - not syncing: Requested init helloworld failed (error -2). ]---
```
le flux de l'affichage ne correspond pas STDOUT

```
cat test.txt | grep ps
[    0.000000] Command line: root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1 init=ps
[    0.070351] Kernel command line: root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1 init=ps
[    0.577756] pps_core: LinuxPPS API ver. 1 registered
[    0.578722] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    1.240160] intel_pstate: CPU model not supported
[    2.015394] Run ps as init process
[    2.015929] Kernel panic - not syncing: Requested init ps failed (error -2).
[    2.025286] ---[ end Kernel panic - not syncing: Requested init ps failed (error -2). ]---
```


```
if [ -z ${KDB} ]; then
    CMDLINE='root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1 init=/usr/bin/bash'
else
    CMDLINE='root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1 kgdbwait'
fi
```


```
[    2.040822] Run /usr/bin/bash as init process
bash: cannot set terminal process group (-1): Inappropriate ioctl for device
bash: no job control in this shell
[root@(none) /]# [    5.262701] ext4lazyinit (50) used greatest stack depth: 14680 bytes left
```

`mount -t proc proc /proc`

Q6.
```
cat init.txt | grep -n init 
80:[    0.041611] mem auto-init: stack:off, heap alloc:off, heap free:off
81:[    0.044610] Memory: 989984K/1048056K available (18432K kernel code, 2727K rwdata, 6476K rodata, 2628K init, 1540K bss, 57812K reserved, 0K cma-reserved)
92:[    0.049928] rcu: srcu_init: Setting srcu_struct sizes based on contention.
113:[    0.233107] LSM: initializing lsm=capability,selinux,integrity
127:[    0.352395] devtmpfs: initialized
132:[    0.357165] audit: initializing netlink subsys (disabled)
135:[    0.359100] audit: type=2000 audit(1707138910.244:1): state=initialized audit_enabled=0 res=1
194:[    0.547333] SCSI subsystem initialized
218:[    0.589907] pnp: PnP ACPI init
314:[    1.207141] device-mapper: ioctl: 4.48.0-ioctl (2023-03-01) initialised: dm-devel@redhat.com
352:[    1.875514] Freeing unused kernel image (initmem) memory: 2628K
358:[    1.976400] Run /sbin/init as init process
373:[    2.652197] random: crng init done
```

## librairie dynamique

