#include "textGen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
words taken from /usr/share/dict/words
bash commands to populate these (assuming $words is the path to ^):
  upper.txt: grep '^[[:upper:]][[:lower:]]\+$' $words | grep -Ee '^.{3,10}$' > \
             words/upper.txt
  lower.txt: grep '^[[:lower:]]\+$' $words | grep -Ee '^.{3,10}$' > \
             words/lower.txt
adjust {3,10} for the length range of the words
*/
static const char LOWER_WORDS_FP[] =
    "/Users/hamburger/Dev/C/typing/words/lower.txt";
static const char UPPER_WORDS_FP[] =
    "/Users/hamburger/Dev/C/typing/words/upper.txt";

char *generateText(struct TextSize *tsz) {
  FILE *lower = fopen(LOWER_WORDS_FP, "r");
  FILE *upper = fopen(UPPER_WORDS_FP, "r");

  struct FileInfo lowerFi, upperFi;
  getFileInfo(lower, &lowerFi);
  getFileInfo(upper, &upperFi);

  char *text = calloc(1, sizeof(char));
  size_t textSize = 0;

  for (int i = 0; i < tsz->paragraphs; i++) {
    // each generateText with {minSntn} - {maxSntn} sentences
    int sntnCount = randRange(tsz->minSntn, tsz->maxSntn);

    for (int j = 0; j < sntnCount; j++) {
      // each sentence with {minWordSntn} - {maxWordSntn} words
      int wordCount = randRange(tsz->minWordSntn, tsz->maxWordSntn);

      for (int k = 0; k < wordCount; k++) {
        char *word;
        if (k == 0)
          // First word
          word = randomWord(upper, &upperFi);
        else
          word = randomWord(lower, &lowerFi);

        textSize = appendStr(&text, textSize, word);
        if (k < wordCount - 1) // no space at the end of a sentence
          textSize = appendStr(&text, textSize, " ");
      }

      textSize = appendStr(&text, textSize, "."); // end of sentence
      if (j < sntnCount - 1) // no space at the end of a paragraph
        textSize = appendStr(&text, textSize, " ");
    }

    if (i < tsz->paragraphs - 1) // no newline after the last paragraph
      textSize = appendStr(&text, textSize, "\n\n"); // end of paragraph
  }

  text[textSize] = '\0';

  fclose(lower);
  fclose(upper);

  return text;
}

// takes a file full of words separated by newline
// outputs a random word from it
char *randomWord(FILE *words, struct FileInfo *fi) {
  // random number between 0 and the last line of the file
  size_t i = randRange(0, fi->lines);
  // jump to that line
  fseek(words, fi->lineOffsets[i], SEEK_SET);

  // surely the word wont be longer than this
  char *word = calloc(64, sizeof(char));
  fgets(word, 64 * sizeof(char), words);

  // delete newline
  size_t size = strlen(word);
  word[size - 1] = '\0';
  word = realloc(word, size);

  return word;
}

// count the number of lines and their byte offset in a file
void getFileInfo(FILE *file, struct FileInfo *fi) {
  size_t line = 0;
  size_t *offsets = calloc(1, sizeof(size_t));
  // temp buffer that just stores a line
  char buffer[64];

  while (!feof(file)) {
    // get line offset and resize array
    offsets[line++] = ftell(file);
    offsets = realloc(offsets, sizeof(size_t) * (line + 1));

    // read the line to move the position indicator
    fgets(buffer, 64, file);
  }

  // fill the struct
  fi->lines = line;
  fi->lineOffsets = offsets;
}

// appends {add} to {str}, returning the new size
size_t appendStr(char **str, size_t size, char *add) {
  // resize str
  size_t appendLen = strlen(add);
  *str = realloc(*str, sizeof(char) * (size + appendLen + 1));

  // copy {add} to the (original) end of {str}
  memcpy(*str + size, add, sizeof(char) * appendLen);

  return size + appendLen;
}

// just a random number within a range
int randRange(int min, int max) { return min + rand() % (max - min + 1); }
