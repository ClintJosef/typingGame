#include <stdlib.h>
#include <string.h>

void shrinkStr(char **str, size_t trim) {
  size_t newSize = strlen(*str) - trim;
  char *new = malloc(sizeof(char) * newSize);
  memcpy(new, *str, newSize);

  *str = new;
}
