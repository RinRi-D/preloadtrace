#include "shared_buf.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
  int fp = open("test.txt", O_WRONLY | O_CREAT, 0666);
  fp = open("test2.txt", O_WRONLY | O_CREAT, 0666);
  fp = open("test3.txt", O_WRONLY | O_CREAT, 0666);
  fp = open("test4.txt", O_WRONLY | O_CREAT, 0666);
  fp = open("test5.txt", O_WRONLY | O_CREAT, 0666);
  // write(fp, "asd\n", strlen("asd\n"));
  // close(fp);
}
