#include "shared_buf.h"

#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef enum { MODE_IO, MODE_MM } Mode;
const char *entry_types[] = {"open",   "close",   "lseek", "read",  "write",
                             "malloc", "realloc", "free",  "repeat"};

static volatile int keepRunning = 1;

void intHandler(int dummy) { keepRunning = 0; }

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec) {
  struct timespec ts;
  int res;

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}

void get_time_str(uint64_t msec, char *tmbuf) {
  struct timeval tv;
  tv.tv_sec = msec / 1000;
  msec %= 1000;
  struct tm *nowtm = gmtime(&tv.tv_sec);

  strftime(tmbuf, 64, "%F %T", nowtm);
  sprintf(tmbuf + strlen(tmbuf), ".%lu", msec);
}

void print_log(Entry *entry, FILE *fp) {
  char tmbuf[64];
  get_time_str(entry->millis, tmbuf);
  fprintf(fp, "%s %s", entry_types[entry->type], tmbuf);
  switch (entry->type) {
  case TYPE_OPEN:
    fprintf(fp, " %s %d %d\n", entry->data.open.pathname,
            entry->data.open.flags, entry->data.open.fd);
    break;
  case TYPE_CLOSE:
    fprintf(fp, " %d %d\n", entry->data.close.fildes, entry->data.close.res);
    break;
  case TYPE_LSEEK:
    fprintf(fp, " %d %lu %d %lu\n", entry->data.lseek.fildes,
            entry->data.lseek.offset, entry->data.lseek.whence,
            entry->data.lseek.res);
    break;
  case TYPE_READ:
    fprintf(fp, " %d %p %lu %ld\n", entry->data.read.fd, entry->data.read.buf,
            entry->data.read.count, entry->data.read.res);
    break;
  case TYPE_WRITE:
    fprintf(fp, " %d %p %lu %ld\n", entry->data.write.fd, entry->data.write.buf,
            entry->data.write.count, entry->data.write.res);
    break;
  case TYPE_MALLOC:
    fprintf(fp, " %lu %p\n", entry->data.malloc.size, entry->data.malloc.res);
    break;
  case TYPE_REALLOC:
    fprintf(fp, " %lu %p %p\n", entry->data.realloc.size,
            entry->data.realloc.prev, entry->data.realloc.res);
    break;
  case TYPE_FREE:
    fprintf(fp, " %p\n", entry->data.free.ptr);
  case TYPE_REPEAT:
    break;
  }
  fflush(fp);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s [io/mm] interval_msec file", argv[0]);
    exit(1);
  }
  long interval = atoi(argv[2]);
  if (interval < 0) {
    fprintf(stderr, "Negative interval");
    exit(1);
  }

  Mode mode;
  if (strncmp(argv[1], "io", 3) == 0) {
    mode = MODE_IO;
  } else if (strncmp(argv[1], "mm", 3) == 0) {
    mode = MODE_MM;
  } else {
    fprintf(stderr, "Usage: %s [io/mm] interval_msec file", argv[0]);
    exit(1);
  }

  DIR *d;
  struct dirent *dir;
  d = opendir("/tmp/preloadtrace");
  if (!d) {
    fprintf(stderr, "No logs");
    exit(1);
  }

  char smode[3];
  if (mode == MODE_IO) {
    memcpy(smode, "io", 3);
  } else {
    memcpy(smode, "mm", 3);
  }

  char pathname[512] = "/tmp/preloadtrace/";
  while ((dir = readdir(d)) != NULL) {
    if (strncmp(dir->d_name, smode, 2) == 0) {
      memcpy(pathname + strlen(pathname), dir->d_name, strlen(dir->d_name) + 1);
      break;
    }
  }
  closedir(d);

  if (mode == MODE_IO) {
    shared_buf_io_init(pathname);
  } else {
    shared_buf_mm_init(pathname);
  }

  signal(SIGINT, intHandler);
  FILE *fp = fopen(argv[3], "a");
  while (keepRunning) {
    Entry entry;
    if (mode == MODE_IO) {
      entry = read_entry_from_io_buf();
    } else {
      entry = read_entry_from_mm_buf();
    }

    if (entry.type == TYPE_REPEAT) {
      msleep(interval);
      continue;
    }

    print_log(&entry, fp);

    msleep(interval);
  }
  fclose(fp);
}
