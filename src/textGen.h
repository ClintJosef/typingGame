#ifndef TEXTGEN_H
#define TEXTGEN_H

#include <stdio.h>

typedef struct {
  size_t lines;
  size_t *lineOffsets;
  size_t *lineLengths;
} WordsInfo;

typedef struct {
  int paragraphs;
  int minSntn;
  int maxSntn;
  int minWordSntn;
  int maxWordSntn;
} TextSize;

char *generateText(int seed, const TextSize *tsz);
char *randomWord(char *words, const WordsInfo *wi);
void getWordsInfo(char *words, WordsInfo *wi);
size_t appendStr(char **text, size_t size, char *add);
size_t randRange(size_t min, size_t max);

#endif
