#include <fcntl.h>
#include <unistd.h>

int main() {
  int fp = open("test", O_WRONLY);
  write(fp, "asd", 3);
}
