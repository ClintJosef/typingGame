#include "ansi.h"

const char *RESET = "\033[0m";
const char *BOLD = "\033[1m";
const char *FAINT = "\033[2m";
const char *UNDERLINE = "\033[4m";
const char *HIDDEN = "\033[?25l";
const char *SHOWN = "\033[?25h";
const char *CUR_HOME = "\033[H";
const char *SAVE_CUR = "\033[s";
const char *RESTORE_CUR = "\033[u";
const char *ERASE_LINE = "\033[2K";
const char *SAVE_SCREEN = "\033[?1049h";
const char *RESTORE_SCREEN = "\033[?1049l";
const char *ERASE_SCREEN = "\033[2J";
const char *ERASE_CUR_TO_END = "\033[0J";
