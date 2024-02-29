#include "shared_buf.h"

#include <fcntl.h>
#include <unistd.h>

int main() {
  int fd = open("buf.bin", O_RDONLY, 0666);

  const size_t sz = sizeof(Entry);
  Entry entry;
  ssize_t r = 0;
  for (int i = 0; i < SHARED_BUF_ENTRIES; i++) {
    r += read(fd, &entry, sz);
    printf("%ld %d %s %d %d\n", r, entry.type, entry.data.open.pathname,
           entry.data.open.flags, entry.data.open.fd);
    lseek(fd, r, SEEK_CUR);
  }
}
