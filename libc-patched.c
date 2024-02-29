#include <dlfcn.h>
#include <stdio.h>

int (*o_open)(const char *, int oflag) = NULL;

int open(const char *pathname, int oflag) {
  if (!o_open)
    o_open = dlsym(RTLD_NEXT, "open");
  printf("%s", pathname);
  return o_open(pathname, oflag);
}
