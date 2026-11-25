#include "movement.h"

// move cursor up/down
size_t moveY(size_t pos, int dir, const struct TypingInfo *typing,
             const struct DisplayInfo *disp) {
  static int savedCol = 0;

  // get user position
  int line = findLine(pos, disp);
  int col = pos - disp->lineOffsets[line];
  if (getLineLength(line, typing->textLen, disp) > savedCol)
    savedCol = col;

  if (dir < 0 && line > 0 && line < disp->lines) {
    // skip trailing newlines (similar to moveX)
    while (getLineLength(line + dir, typing->textLen, disp) < 2)
      dir--;
  }

  // move to the saved column if the line is long enough
  // otherwise just move to the end of that line
  int newLine = wrap(line + dir, 0, disp->lines);
  int lineLen = getLineLength(newLine, typing->textLen, disp);
  if (lineLen < col && lineLen > 1) {
    savedCol = col;
    col = lineLen - 1;
    while (typing->text[disp->lineOffsets[newLine] + col] == '\n')
      col--;
  } else if (col != 0)
    col = savedCol;

  return disp->lineOffsets[newLine] + col;
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
