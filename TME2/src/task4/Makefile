# Question 2
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
#export LD_LIBRARY_PATH=/home/siyuanchen/Documents/PNL_2024/TME2/src/task4:$LD_LIBRARY_PATH
#export LD_PRELOAD=./libread.so ./cron_func

all: cron_func

cron_func: cron_func.o
	$(CC) -o $@ cron_func.o -L. -lfunc -lread

cron_func.o: cron_func.c func.h
	$(CC) $(CFLAGS) -c $< -o $@

libfunc.so: nothing.c func.h
	$(CC) $(LDFLAGS) $(CFLAGS) -o libfunc.so nothing.c

# Question 3
libread.so: read.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o libread.so read.c

.PHONY: clean

clean:
	rm -f cron_func read.o cron_func.o libfunc.so libread.so nothing.o
