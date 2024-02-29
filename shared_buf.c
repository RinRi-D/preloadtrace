#include "shared_buf.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IPC_RESULT_ERROR (-1)

size_t pos = 0;
char *buf = NULL;

void shared_buf_init() {
  buf = attach_memory_block("/tmp/testing-shm", SHARED_BUF_SIZE);
  if (buf == NULL) {
    exit(1);
  }
}

static void write_entry_to_buf(Entry *entry) {
  // TODO async write
  if (buf == NULL)
    return;
  memcpy(buf + pos, entry, sizeof(Entry));
  pos += sizeof(Entry);
  if (pos == SHARED_BUF_SIZE) {
    pos = 0;
  }
}

Entry read_entry_from_buf() {
  Entry entry;
  printf("pos %lu\n", pos);
  memcpy(&entry, buf + pos, sizeof(Entry));
  if (entry.millis == 0 && pos > 0) {
    memcpy(&entry, buf + pos - sizeof(Entry), sizeof(Entry));
    if (entry.millis == 0) {
      pos = 0;
      return entry;
    }
  }

  pos += sizeof(Entry);
  if (pos == SHARED_BUF_SIZE) {
    pos = 0;
  }
  return entry;
}

void dump_buf() {
  if (buf != NULL) {
    int fd = open("buf.bin", O_WRONLY | O_CREAT, 0666);
    write(fd, buf, sizeof(Entry) * SHARED_BUF_ENTRIES);
  }
}

void append_entry(Entry *entry) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  entry->millis = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;

  write_entry_to_buf(entry);
}

static int get_shared_block(char *pathname, size_t size) {
  key_t key;

  FILE *fp = fopen(pathname, "ab+");
  fclose(fp);
  key = ftok(pathname, 1);
  if (key == IPC_RESULT_ERROR) {
    return IPC_RESULT_ERROR;
  }

  return shmget(key, size, 0666 | IPC_CREAT);
}

char *attach_memory_block(char *pathname, size_t size) {
  int shared_block_id = get_shared_block(pathname, size);
  if (shared_block_id == IPC_RESULT_ERROR) {
    return NULL;
  }

  char *res = shmat(shared_block_id, NULL, 0);
  if (res == (char *)IPC_RESULT_ERROR) {
    return NULL;
  }

  return res;
}

char detach_memory_block(char *block) {
  return (shmdt(block) != IPC_RESULT_ERROR);
}

char destroy_memory_block(char *pathname) {
  int shared_block_id = get_shared_block(pathname, 0);
  if (shared_block_id == IPC_RESULT_ERROR) {
    return NULL;
  }

  return (shmctl(shared_block_id, IPC_RMID, NULL) != IPC_RESULT_ERROR);
}
