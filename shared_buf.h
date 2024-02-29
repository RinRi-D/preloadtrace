#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>

// dont change separately!
#define SHARED_BUF_ENTRIES 128
#define SHARED_BUF_SIZE 527360
// #define SHARED_BUF_SIZE (SHARED_BUF_ENTRIES * sizeof(Entry))

typedef enum {
  TYPE_OPEN,
  TYPE_CLOSE,
  TYPE_LSEEK,
  TYPE_READ,
  TYPE_WRITE,
  TYPE_MALLOC,
  TYPE_REALLOC,
  TYPE_FREE,
} EntryType;

typedef struct {
  // PATH_MAX
  char pathname[4096];
  int flags;
  int fd;
} EntryOpen;

typedef struct {
  int fildes;
  int res;
} EntryClose;

typedef struct {
  int fildes;
  unsigned long offset;
  int whence;
  unsigned long res;
} EntryLseek;

typedef struct {
  int fd;
  void *buf;
  size_t count;
  ssize_t res;
} EntryRead;

typedef struct {
  int fd;
  const void *buf;
  size_t count;
  ssize_t res;
} EntryWrite;

typedef struct {
  size_t size;
  void *res;
} EntryMalloc;

typedef struct {
  size_t size;
  void *prev;
  void *res;
} EntryRealloc;

typedef struct {
  void *ptr;
} EntryFree;

typedef struct {
  uint64_t millis;
  EntryType type;
  union {
    EntryOpen open;
    EntryClose close;
    EntryLseek lseek;
    EntryRead read;
    EntryWrite write;
    EntryMalloc malloc;
    EntryRealloc realloc;
    EntryFree free;
  } data;
} Entry;

void shared_buf_init();
void append_entry(Entry *entry);
Entry read_entry_from_buf();
void dump_buf();
char *attach_memory_block(char *pathname, size_t size);
char detach_memory_block(char *block);
char destroy_memory_block(char *pathname);
