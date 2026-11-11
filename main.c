#include "setup.h"
#include <ctype.h>
#include <stdio.h>

const char RESET[] = "\e[0m";
const char BOLD[] = "\e[1m";
const char FAINT[] = "\e[2m";

int main() {
  if (init() == -1)
    return 1;

  // code
  char c;
  while (tolower(c) != 'q') {
    c = getchar();
    printf("%c", c);
  }

  if (cleanup() == -1)
    return 1;
}
