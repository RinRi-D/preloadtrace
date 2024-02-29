#include "shared_buf.h"

#include <unistd.h>

int main() {
  shared_buf_init();
  for (int i = 0; i < 128; ++i) {
    Entry entry = read_entry_from_buf();
    printf("%d %lu", entry.type, entry.millis);
    if (entry.type == 0) {
      printf(" %s", entry.data.open.pathname);
    }
    printf("\n");
  }
}
