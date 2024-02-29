CC=gcc

all: a.out libc-patched.so read-daemon.out

a.out: a.c shared_buf.c
	$(CC) -Wall -g -o a.out a.c shared_buf.c

read-daemon.out: read-daemon.c shared_buf.c
	$(CC) -Wall -g -o read-daemon.out read-daemon.c shared_buf.c

libc-patched.so: libc-patched.c shared_buf.c
	$(CC) -Wall -g -fPIC -shared -o libc-patched.so libc-patched.c shared_buf.c

run:
	LD_PRELOAD=./libc-patched.so ./a.out

clean:
	rm -f *.out *.so *.o
