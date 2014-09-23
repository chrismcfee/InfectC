//header for printing functions
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "include.h"
//camelcase ftw!
void initNcurses();
int displayBoard(const Board * const * const board);
void printError(const char *error);
void printHelp();
void printVersion();
void print_in_middle(WINDOW *win, const int starty, const int startx, int width, const char *string, const chtype color);

#endif
