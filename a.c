#include "shared_buf.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
  int fp = open("test.txt", O_WRONLY | O_CREAT, 0666);
  write(fp, "asd\n", strlen("asd\n"));
  close(fp);
  char *b = malloc(100);
  char *c = malloc(100);
  b = realloc(b, 1000);
  free(b);
  free(c);

  fp = open("test.txt", O_RDONLY);
  lseek(fp, 1, SEEK_CUR);
  char buf[2];
  read(fp, buf, 2);
  close(fp);
}
