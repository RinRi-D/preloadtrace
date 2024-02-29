#define _GNU_SOURCE

#include "shared_buf.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  shared_buf_init();
  printf("Called setup!");
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
  }
  { res = o_open(pathname, flags); }

  // printf("open: %s %d %d\n", pathname, flags, res);
  Entry entry;
  entry.type = TYPE_OPEN;
  memcpy(entry.data.open.pathname, pathname, strlen(pathname));
  entry.data.open.flags = flags;
  entry.data.open.fd = res;
  append_entry(&entry);
  return res;
}

int close(int fildes) {
  if (!o_close)
    o_close = dlsym(RTLD_NEXT, "close");
  int res = o_close(fildes);
  // printf("close: %d %d\n", fildes, res);
  return res;
}

off_t lseek(int fildes, off_t offset, int whence) {
  if (!o_lseek)
    o_lseek = dlsym(RTLD_NEXT, "lseek");
  off_t res = o_lseek(fildes, offset, whence);
  // printf("lseek: %d %lu %d %lu\n", fildes, offset, whence, res);
  return res;
}

ssize_t read(int fd, void *buf, size_t count) {
  if (!o_read)
    o_read = dlsym(RTLD_NEXT, "read");
  ssize_t res = o_read(fd, buf, count);
  // printf("read: %d %p %lu %lu\n", fd, buf, count, res);
  return res;
}

ssize_t write(int fd, const void *buf, size_t count) {
  if (!o_write)
    o_write = dlsym(RTLD_NEXT, "write");
  ssize_t res = o_write(fd, buf, count);
  // printf("write: %d %p %lu %lu\n", fd, buf, count, res);
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

  // doesn't work with malloc -> recursive
  append_entry(&entry);
  return res;
}

void *realloc(void *ptr, size_t size) {
  if (!o_realloc)
    o_realloc = dlsym(RTLD_NEXT, "realloc");
  void *prev = ptr;
  void *res = o_realloc(ptr, size);
  return res;
}

void free(void *ptr) {
  if (!o_free)
    o_free = dlsym(RTLD_NEXT, "free");
  o_free(ptr);
}
