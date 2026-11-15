#include "textGen.h"
#include "output/lower.h"
#include "output/upper.h"
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

char *generateText(struct TextSize *tsz) {
  char *lower = (char *)words_lower_txt;
  char *upper = (char *)words_upper_txt;

  struct WordsInfo lowerFi, upperFi;
  getWordsInfo(lower, &lowerFi);
  getWordsInfo(upper, &upperFi);

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

  return text;
}

// takes a char array of words separated by newline
// outputs a random word from it
char *randomWord(char *words, struct WordsInfo *wi) {
  size_t line = randRange(0, wi->lines - 1);
  size_t offset = wi->lineOffsets[line];
  size_t wordSize = wi->lineLengths[line];

  char *word = calloc(wordSize + 1, sizeof(char));

  memcpy(word, &words[offset], wordSize);

  return word;
}

// count the number of lines and their byte offset in a char array
void getWordsInfo(char *words, struct WordsInfo *wi) {
  size_t line = 0;
  size_t *offsets = calloc(1, sizeof(size_t));
  size_t *lineLengths = calloc(1, sizeof(size_t));

  // split words into "tokens" separated by newline, then iterate
  // through each token
  char *wordPtr = strtok(words, "\n");
  while (wordPtr != NULL) { // NULL when there arent any tokens left
    // increase array sizes
    offsets = realloc(offsets, (line + 1) * sizeof(size_t));
    lineLengths = realloc(lineLengths, (line + 1) * sizeof(size_t));

    // woah, pointer arithmetic
    // wordPtr is just words advanced by the offset
    // so we get the offset by subtracting words again
    offsets[line] = wordPtr - words;

    size_t wordSize = strlen(wordPtr);
    lineLengths[line] = wordSize;

    // go to next token
    wordPtr = strtok(NULL, "\n");
    line++;
  }

  // fill the struct
  wi->lines = line;
  wi->lineOffsets = offsets;
  wi->lineLengths = lineLengths;
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
