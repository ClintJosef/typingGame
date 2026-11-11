#include <stdlib.h>
#include <string.h>

void shrinkStr(char **str, size_t size, size_t trim) {
  size_t newSize = sizeof(char) * (size - trim);
  char *new = malloc(newSize);
  memcpy(new, *str, newSize);

  *str = new;
}
