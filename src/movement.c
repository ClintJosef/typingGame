#include "movement.h"

// move cursor up/down
size_t moveY(size_t pos, int dir, const struct TypingInfo *typing,
             const struct DisplayInfo *disp) {
  static int savedCol = 0;
  static int currPage = 0;

  size_t textLen = typing->textLen;

  // line range of the current page
  int page = typing->currPage;
  int firstLine = findLine(disp->pageOffsets[page], disp);
  int lastLine = firstLine + disp->pageLines[page];

  // user position
  size_t line = findLine(pos, disp);
  int col = pos - disp->lineOffsets[line];
  int lineLen = getLineLength(line, textLen, disp);
  if (lineLen > savedCol)
    savedCol = col;

  // save column only within the same page
  if (currPage != page)
    savedCol = 0;
  currPage = page;

  // go to next/prev line thats not just a newline char
  int newLine = line;
  lineLen = 0;
  do {
    newLine = wrap(newLine + dir, firstLine, lastLine);
    lineLen = getLineLength(newLine, textLen, disp);
  } while (lineLen < 2);

  // move to end of line if line is too short, otherwise to savedCol
  if (lineLen < savedCol) {
    savedCol = col;
    col = lineLen - 2;
  } else
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
