#include "shared_buf.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IPC_RESULT_ERROR (-1)

size_t io_pos = 0;
size_t mm_pos = 0;
char *io_buf = NULL;
char *mm_buf = NULL;

void shared_buf_io_init(char *pathname) {
  io_buf = attach_memory_block(pathname, SHARED_BUF_SIZE);
  if (io_buf == NULL) {
    exit(1);
  }
}

void shared_buf_mm_init(char *pathname) {
  mm_buf = attach_memory_block(pathname, SHARED_BUF_SIZE);
  if (mm_buf == NULL) {
    exit(1);
  }
}

static void write_entry_to_io_buf(Entry *entry) {
  // TODO async write
  if (io_buf == NULL)
    return;
  memcpy(io_buf + io_pos, entry, sizeof(Entry));
  io_pos += sizeof(Entry);
  if (io_pos == SHARED_BUF_SIZE) {
    io_pos = 0;
  }
}

static void write_entry_to_mm_buf(Entry *entry) {
  // TODO async write
  if (mm_buf == NULL)
    return;
  memcpy(mm_buf + mm_pos, entry, sizeof(Entry));
  mm_pos += sizeof(Entry);
  if (mm_pos == SHARED_BUF_SIZE) {
    mm_pos = 0;
  }
}

Entry read_entry_from_io_buf() {
  Entry entry;
  memcpy(&entry, io_buf + io_pos, sizeof(Entry));
  if (entry.millis == 0 && io_pos > 0) {
    memcpy(&entry, io_buf + io_pos - sizeof(Entry), sizeof(Entry));
    entry.type = TYPE_REPEAT;
    if (entry.millis == 0) {
      io_pos = 0;
      return entry;
    }
    return entry;
  }

  io_pos += sizeof(Entry);
  if (io_pos == SHARED_BUF_SIZE) {
    io_pos = 0;
  }
  return entry;
}

Entry read_entry_from_mm_buf() {
  Entry entry;
  memcpy(&entry, mm_buf + mm_pos, sizeof(Entry));
  if (entry.millis == 0 && mm_pos > 0) {
    memcpy(&entry, mm_buf + mm_pos - sizeof(Entry), sizeof(Entry));
    entry.type = TYPE_REPEAT;
    if (entry.millis == 0) {
      mm_pos = 0;
      return entry;
    }
    return entry;
  }

  mm_pos += sizeof(Entry);
  if (mm_pos == SHARED_BUF_SIZE) {
    mm_pos = 0;
  }
  return entry;
}

void append_entry(Entry *entry) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  entry->millis = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;

  if (entry->type == TYPE_MALLOC || entry->type == TYPE_REALLOC ||
      entry->type == TYPE_FREE) {
    write_entry_to_mm_buf(entry);
  } else {
    write_entry_to_io_buf(entry);
  }
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
    return IPC_RESULT_ERROR;
  }

  return (shmctl(shared_block_id, IPC_RMID, NULL) != IPC_RESULT_ERROR);
}
