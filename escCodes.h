#include <stdbool.h>
#include <stdio.h>

const char RESET[] = "\e[0m";
const char BOLD[] = "\e[1m";
const char FAINT[] = "\e[2m";
const char CUR_HOME[] = "\e[H";
const char SAVE_CUR[] = "\e[s";
const char RESTORE_CUR[] = "\e[u";

struct Colors {
  char *untyped;
  char *incorrect;
  char *correct;
};

const struct Colors COLORS = {"\e[2m", "\e[31m", "\e[38;5;76m"};

void displayText(const char *text, bool faint, bool restore) {
  const char *mod = faint ? FAINT : "";
  printf("%s%s%s%s", SAVE_CUR, mod, text, RESET);
  if (restore)
    printf("%s", RESTORE_CUR);
}

void moveCursor(int row, int col) { printf("\e[%d;%df", row, col); }
void moveToCol(int col) { printf("\e[%dG", col); }
void clearLine() { printf("\e[2K"); }
