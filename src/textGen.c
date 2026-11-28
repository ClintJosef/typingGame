#include "textGen.h"
#include "words/lower.h"
#include "words/upper.h"
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

// generate text of a specified size with a given seed
char *generateText(int seed, const struct TextSize *tsz) {
  // .h files generated in output/
  char *lower = (char *)words_lower_txt;
  char *upper = (char *)words_upper_txt;

  struct WordsInfo lowerFi, upperFi;
  getWordsInfo(lower, &lowerFi);
  getWordsInfo(upper, &upperFi);

  char *text = calloc(1, sizeof(char));
  size_t textSize = 0;

  // set seed for random
  srand(seed);

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
char *randomWord(char *words, const struct WordsInfo *wi) {
  size_t line = randRange(0, wi->lines - 1);
  size_t offset = wi->lineOffsets[line];
  size_t wordSize = wi->lineLengths[line];

  // im assuming the word pulled doesnt include the null byte?
  char *word = calloc(wordSize + 1, sizeof(char));
  memcpy(word, &words[offset], wordSize);

  return word;
}

// count the number of lines and their byte offset in a char array
void getWordsInfo(char *words, struct WordsInfo *wi) {
  size_t capac = 16;
  size_t line = 0;
  size_t *offsets = calloc(capac, sizeof(size_t));
  size_t *lineLengths = calloc(capac, sizeof(size_t));

  // split words into "tokens" separated by newline, then iterate
  // through each token
  char *wordPtr = strtok(words, "\n");
  while (wordPtr != NULL) { // NULL when there arent any tokens left
    // increase array sizes
    if (line == capac) {
      capac *= 2;
      offsets = realloc(offsets, capac * sizeof(size_t));
      lineLengths = realloc(lineLengths, capac * sizeof(size_t));
    }

    // woah, pointer arithmetic
    // wordPtr is just words advanced by the offset
    // so we get the offset by subtracting words again
    offsets[line] = wordPtr - words;
    lineLengths[line] = strlen(wordPtr);

    // advance to next token (line)
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

  // copy {add} to the end of {str}
  memcpy(*str + size, add, sizeof(char) * appendLen);

  return size + appendLen;
}

// just a random number within a range
size_t randRange(size_t min, size_t max) {
  return min + rand() % (max - min + 1);
}
