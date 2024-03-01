#define _GNU_SOURCE

#include "shared_buf.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int (*o_open)(const char *, int oflag, ...) = NULL;
int (*o_close)(int fildes) = NULL;
off_t (*o_lseek)(int fildes, off_t offset, int whence) = NULL;
ssize_t (*o_read)(int fd, void *buf, ssize_t count) = NULL;
ssize_t (*o_write)(int fd, const void *buf, ssize_t count) = NULL;
void *(*o_malloc)(size_t size) = NULL;
void *(*o_realloc)(void *ptr, size_t size) = NULL;
void (*o_free)(void *ptr) = NULL;

__attribute__((constructor)) static void setup(void) {
  int len;
  pid_t pid = getpid();
  struct stat st = {0};

  if (stat("/tmp/preloadtrace", &st) == -1) {
    mkdir("/tmp/preloadtrace", 0755);
  }
  char io_pathname[64] = "/tmp/preloadtrace/io-";

  len = strlen(io_pathname);
  sprintf(io_pathname + len, "%d", pid);

  char mm_pathname[64] = "/tmp/preloadtrace/mm-";
  len = strlen(mm_pathname);
  sprintf(mm_pathname + len, "%d", pid);

  shared_buf_io_init(io_pathname);
  shared_buf_mm_init(mm_pathname);
}

int open(const char *pathname, int flags, ...) {
  if (!o_open)
    o_open = dlsym(RTLD_NEXT, "open");

  int res;
  if (flags & O_CREAT) {
    va_list args;
    va_start(args, flags);
    int arg = va_arg(args, int);
    va_end(args);
    res = o_open(pathname, flags, arg);
  } else {
    res = o_open(pathname, flags);
  }

  Entry entry;
  entry.type = TYPE_OPEN;
  memcpy(entry.data.open.pathname, pathname, strlen(pathname) + 1);
  entry.data.open.flags = flags;
  entry.data.open.fd = res;

  append_entry(&entry);
  return res;
}

int close(int fildes) {
  if (!o_close)
    o_close = dlsym(RTLD_NEXT, "close");
  int res = o_close(fildes);

  Entry entry;
  entry.type = TYPE_CLOSE;
  entry.data.close.fildes = fildes;

  append_entry(&entry);
  return res;
}

off_t lseek(int fildes, off_t offset, int whence) {
  if (!o_lseek)
    o_lseek = dlsym(RTLD_NEXT, "lseek");
  off_t res = o_lseek(fildes, offset, whence);
  // printf("lseek: %d %lu %d %lu\n", fildes, offset, whence, res);

  Entry entry;
  entry.type = TYPE_LSEEK;
  entry.data.lseek.fildes = fildes;
  entry.data.lseek.offset = offset;
  entry.data.lseek.whence = whence;

  append_entry(&entry);
  return res;
}

ssize_t read(int fd, void *buf, size_t count) {
  if (!o_read)
    o_read = dlsym(RTLD_NEXT, "read");
  ssize_t res = o_read(fd, buf, count);

  Entry entry;
  entry.type = TYPE_READ;
  entry.data.read.fd = fd;
  entry.data.read.buf = buf;
  entry.data.read.count = count;

  append_entry(&entry);
  return res;
}

ssize_t write(int fd, const void *buf, size_t count) {
  if (!o_write)
    o_write = dlsym(RTLD_NEXT, "write");
  ssize_t res = o_write(fd, buf, count);

  Entry entry;
  entry.type = TYPE_WRITE;
  entry.data.write.fd = fd;
  entry.data.write.buf = buf;
  entry.data.write.count = count;
  entry.data.write.res = res;

  append_entry(&entry);
  return res;
}

void *malloc(size_t size) {
  if (!o_malloc)
    o_malloc = dlsym(RTLD_NEXT, "malloc");
  void *res = o_malloc(size);

  Entry entry;
  entry.type = TYPE_MALLOC;
  entry.data.malloc.size = size;
  entry.data.malloc.res = res;

  append_entry(&entry);
  return res;
}

void *realloc(void *ptr, size_t size) {
  if (!o_realloc)
    o_realloc = dlsym(RTLD_NEXT, "realloc");
  void *prev = ptr;
  void *res = o_realloc(ptr, size);

  Entry entry;
  entry.type = TYPE_REALLOC;
  entry.data.realloc.size = size;
  entry.data.realloc.prev = prev;
  entry.data.realloc.res = res;

  append_entry(&entry);

  return res;
}

void free(void *ptr) {
  if (!o_free)
    o_free = dlsym(RTLD_NEXT, "free");

  Entry entry;
  entry.type = TYPE_FREE;
  entry.data.free.ptr = ptr;

  append_entry(&entry);

  o_free(ptr);
}
