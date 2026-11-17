#ifndef TEXTGEN_H
#define TEXTGEN_H

#include <stdio.h>

struct WordsInfo {
  size_t lines;
  size_t *lineOffsets;
  size_t *lineLengths;
};

struct TextSize {
  int paragraphs;
  int minSntn;
  int maxSntn;
  int minWordSntn;
  int maxWordSntn;
};

char *generateText(const struct TextSize *tsz);
char *randomWord(char *words, const struct WordsInfo *wi);
void getWordsInfo(char *words, struct WordsInfo *wi);
int randRange(int min, int max);
size_t appendStr(char **text, size_t size, char *add);

#endif
