#include "args.h"
#include <getopt.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>

static char *progName = "typingGame";

// read stdin first, if its empty then parse the args
char *parseArgs(int argc, char *argv[], int defaultSeed,
                const struct TextSize *defaultTsz) {
  // get program name (in case it was renamed)
  progName = basename(argv[0]);

  if (!isatty(STDIN_FILENO)) {
    // try to read from stdin
    char *input = readStdin();
    if (input != NULL)
      return input;
  }

  // copy of the defaults
  struct TextSize tsz = *defaultTsz;
  int seed = defaultSeed;

  // loop through every option given
  int opt;
  bool showHelp = false;
  while ((opt = getopt(argc, argv, "p:s:S:w:W:x:h")) != -1) {
    switch (opt) {
    case 'p':
      tsz.paragraphs = parseIntOrExit(optarg);
      break;
    case 's':
      tsz.minSntn = parseIntOrExit(optarg);
      break;
    case 'S':
      tsz.maxSntn = parseIntOrExit(optarg);
      break;
    case 'w':
      tsz.minWordSntn = parseIntOrExit(optarg);
      break;
    case 'W':
      tsz.maxWordSntn = parseIntOrExit(optarg);
      break;
    case 'x':
      seed = parseIntOrExit(optarg);
      break;
    case 'h':
      showHelp = true;
      break;
    case '?':
    default:
      usage(true);
    }
  }

  if (showHelp)
    usage(false);
  validateTsz(&tsz);

  return generateText(seed, &tsz);
}

char *readStdin(void) {
  // initial size
  size_t capac = 1024;
  size_t len = 0;
  char *buffer = calloc(capac, sizeof(char));

  // read n bytes until nothing can be read
  // append new data to the end (buffer + len)
  // and read as much as can fit (capac - len)
  size_t n;
  while ((n = fread(buffer + len, sizeof(char), capac - len, stdin)) > 0) {
    len += n;

    // dynamically resize
    if (len >= capac) {
      capac *= 2;
      buffer = realloc(buffer, sizeof(char) * capac);
    }
  }

  // something went wrong
  if (ferror(stdin) || len == 0) {
    free(buffer);
    return NULL;
  }

  buffer[len] = '\0';
  return buffer;
}

// exit program if user passed in invalid numbers as arguments
int parseIntOrExit(char *str) {
  // points to the next invalid character in the str
  // if none, points to the end of the string ('\0')
  // if it couldnt convert anything, this just ends up as the original string
  char *endPtr;

  int num = strtol(str, &endPtr, 10);

  // since im only going to use this here ill just validate positive values
  if (endPtr == str || *endPtr != '\0' || num < 1)
    usage(true);

  return num;
}

// tsz must have valid values, otherwise show the help message
void validateTsz(const struct TextSize *tsz) {
  int paragraphs = tsz->paragraphs;
  int minSntn = tsz->minSntn;
  int maxSntn = tsz->maxSntn;
  int minWordSntn = tsz->minWordSntn;
  int maxWordSntn = tsz->maxWordSntn;

  bool anyNegative = paragraphs < 0 || minSntn < 0 || maxSntn < 0 ||
                     minWordSntn < 0 || maxWordSntn < 0;
  bool invalidRange = minSntn > maxSntn || minWordSntn > maxWordSntn;

  if (anyNegative || invalidRange)
    usage(true);
}

// help message
void usage(bool invalid) {
  static const char MSG[] =
    "Usage: %s [-p paragraphs] [-s sentences] [-S sentences] [-w words] [-W words] [-x seed] [-h]\n\n"
    "A typing game in the terminal.\n"
    "Navigate the text via arrow keys (cursor) and '[' or ']' (pages).\n\n"
    "Options:\n"
    "    -p <paragraphs>\t:no. of paragraphs\t\t\t(default: 2)\n"
    "    -s <sentences>\t:minimum no. of sentences per paragraph\t(default: 3)\n"
    "    -S <sentences>\t:maximum no. of sentences per paragraph\t(default: 5)\n"
    "    -w <words>\t\t:minimum no. of words per sentence\t(default: 4)\n"
    "    -W <words>\t\t:maximum no. of words per sentence\t(default: 6)\n"
    "    -x <seed>\t\t:seed to generate text\t\t\t(default: system time)\n"
    "    -h \t\t\t:shows this help message\n\n"
    "Options are for the built-in text generator.\n"
    "Alternatively, you can pipe your own text through stdin\n";

  if (invalid)
    fprintf(stderr, "Invalid options.\n");
  fprintf(stderr, MSG, progName);

  exit(invalid);
}
