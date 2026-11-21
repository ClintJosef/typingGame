#include "args.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

// read stdin first, if its empty then parse the args
char *parseArgs(int argc, char *argv[], const struct TextSize *defaultTsz) {
  if (argc < 2) {
    if (!isatty(STDIN_FILENO)) {
      // try to read from stdin
      char *input = readStdin();
      if (input != NULL)
        return input;
    }

    // no input/couldnt read from stdin -> generate default text
    return generateText(defaultTsz);
  }

  // all the options
  // the number is just its "id" returned by getopt_long_only etc.
  // last option has to be all 0s for some reason
  static struct option longOpts[] = {
      {"pg", optional_argument, NULL, 1},
      {"smin", optional_argument, NULL, 2},
      {"smax", optional_argument, NULL, 3},
      {"wmin", optional_argument, NULL, 4},
      {"wmax", optional_argument, NULL, 5},
      {"help", no_argument, NULL, 'h'},
      {"h", no_argument, NULL, 'h'},
      {0, 0, 0, 0},
  };

  // copy of the defaults
  struct TextSize tsz = *defaultTsz;

  int opt;
  // loop through every option given
  while ((opt = getopt_long_only(argc, argv, "", longOpts, NULL)) != -1) {
    switch (opt) {
    case 1:
      tsz.paragraphs = parseIntOrExit(optarg);
      break;
    case 2:
      tsz.minSntn = parseIntOrExit(optarg);
      break;
    case 3:
      tsz.maxSntn = parseIntOrExit(optarg);
      break;
    case 4:
      tsz.minWordSntn = parseIntOrExit(optarg);
      break;
    case 5:
      tsz.maxWordSntn = parseIntOrExit(optarg);
      break;
    case 'h':
      usage(false);
      break;
    case '?':
    default:
      usage(true);
    }
  }

  if (tsz.maxSntn < tsz.minSntn || tsz.maxWordSntn < tsz.minWordSntn)
    usage(true);

  return generateText(&tsz);
}

char *readStdin(void) {
  // initial size
  size_t capac = 1024;
  size_t len = 0;
  char *buffer = calloc(capac, sizeof(char));

  // read n bytes until nothing can be read
  size_t n;
  while ((n = fread(buffer, sizeof(char), capac - len - 1, stdin)) > 0) {
    // dynamically resize
    len += n;
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

// help message
void usage(bool invalid) {
  static const char MSG[] =
      "Usage: typingGame [-pg -smin -smax -wmin -wmax -h -help]\n\n"
      "Options:\n"
      "    -pg \t:no. of paragraphs\t\t\t(default: 2)\n"
      "    -smin \t:minimum no. of sentences per paragraph\t(default: 3)\n"
      "    -smax \t:maximum no. of sentences per paragraph\t(default: 5)\n"
      "    -wmin \t:minimum no. of words per sentence\t(default: 4)\n"
      "    -wmax \t:maximum no. of words per sentence\t(default: 6)\n"
      "    -h/-help \t:prints this help message\n\n"
      "Each arg must be passed either as '-arg=value' or '--arg=value'\n"
      "You can also pipe another program's output into this, but it won't be "
      "formatted\n";

  if (invalid)
    fprintf(stderr, "Invalid args.\n\n");
  fprintf(stderr, "%s", MSG);

  exit(invalid);
}
