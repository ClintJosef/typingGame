#ifndef TEXTGEN_H
#define TEXTGEN_H

#include <stdio.h>

struct FileInfo {
  size_t lines;        // num of lines in a file
  size_t *lineOffsets; // byte offset of each line
};

struct TextSize {
  int paragraphs;
  int minSntn;
  int maxSntn;
  int minWordSntn;
  int maxWordSntn;
};

char *generateText(struct TextSize *tsz);
char *randomWord(FILE *words, struct FileInfo *fi);
void getFileInfo(FILE *file, struct FileInfo *fi);
int randRange(int min, int max);
size_t appendStr(char **text, size_t size, char *add);

#endif
