change paramètre

```
insmod helloworld.ko
cd /sys/module/helloworld/parameters
nano whom
-> change nom
rmmod helloworld
```

trouver cette structure via elixir

grep -rn "init_uts_ns" /repertoire root du kernel/


EXPORT_SYMBOL(modules) dans /scripts/mod/modpost.c


