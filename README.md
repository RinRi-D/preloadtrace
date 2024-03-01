# Preloadtrace

A library and a daemon that override some functions (open(), close(), malloc(), etc) by utilizing LD_PRELOAD to log functions' arguments and results.

## Run & test

### Patched program

```bash
make clean
make
LD_PRELOAD=./libc-patched.so ./a.out
```

### Daemon
```bash
# assuming you run make clean and make
# Usage: ./read-daemon.out [io/mm] interval_msec file
./read-daemon.out mm 0 test.log
```

Note: it's important to run make clean or at least rm -f /tmp/preloadtrace/* so that the files from previous runs are not kept. (Haven't implemented deletion and multi processes support yet)

## Progress & Notes

[notes.org](notes.org)
