#ifndef GENERATION_H_
#define GENERATION_H_

#include "include.h"

// This header provides functions to initialise the game variables and generate the game board.
// parseargs is called at start via cml args, no menu
Board ** parseArgs(const int argc, const char * const * const argv);
// initDefault() initializes the default board using the game variables.
Board **initDefault();
#endif

