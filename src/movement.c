#include "movement.h"

// move cursor up/down
size_t moveY(size_t pos, int dir, const struct TypingInfo *typing,
             const struct DisplayInfo *disp) {
  static int savedCol = 0;

  // get user position
  int currLine = findLine(pos, disp);
  int currCol = pos - disp->lineOffsets[currLine];
  if (getLineLength(currLine, typing->textLen, disp) > savedCol)
    savedCol = currCol;

  if (dir < 0 && currLine > 0 && currLine < disp->lines) {
    // skip preceeding newlines (similar to moveX)
    while (getLineLength(currLine + dir, typing->textLen, disp) < 2)
      dir--;
  }

  // move to the saved column if the line is long enough
  // otherwise just move to the end of that line
  int newLine = wrap(currLine + dir, 0, disp->lines);
  int lineLen = getLineLength(newLine, typing->textLen, disp);
  if (lineLen < currCol && lineLen > 1) {
    savedCol = currCol;
    currCol = lineLen - 1;
    while (typing->text[disp->lineOffsets[newLine] + currCol] == '\n')
      currCol--;
  } else if (currCol != 0)
    currCol = savedCol;

  return disp->lineOffsets[newLine] + currCol;
}

// move cursor left/right
size_t moveX(size_t pos, int dir, const struct TypingInfo *typing,
             const struct DisplayInfo *disp) {
  // skip trailing newlines when moving left
  // dont need to skip leading newlines
  // since thats handled in the main loop
  if (dir < 0)
    while (pos + dir > 0 && typing->text[pos + dir] == '\n')
      dir--;

  size_t pageOffset = disp->pageOffsets[typing->currPage];
  return wrap(pos + dir, pageOffset, pageOffset + typing->pageSize - 1);
}

// wrap around a range using modulo
// didnt know where else to put this
int wrap(int a, int min, int max) {
  int range = max - min + 1;
  /*
  1. (a - min) % range ensures a > -range
  2. + range ensures a >= 0
  3. % range ensures 0 <= a < range
  4. + min ensures min <= a < range + min

  since range is inclusive of max, this becomes:
  min <= a <= max
  */
  return min + ((a - min) % range + range) % range;
}
