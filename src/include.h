#ifndef INCLUDE_H_
#define INCLUDE_H_
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>

typedef enum _characters {EMPTY, DEAD, INF, DOC, CIT, SOL, NUR, WALL} Characters;
typedef enum _speeds {SLOW = 10, FAST = 5, FASTEST = 1} Speeds;

typedef struct _board {
	Characters character;
	int direction;
} Board;

typedef struct _units {
	int doctors;
	int infected;
	int nurses;
	int soldiers;
	int citizens;
	int dead;
	int wood;
	int total;
} Units;

typedef struct _dimensions {
	int x;
	int y;
} Dimensions;

typedef struct _time {
	int elapsed;
	const int timeout;
	int days;
	int refreshRate;
	int steps;
} Time;

typedef struct _variables {
	Units units;
	Dimensions dim;
	Time time;
} Variables;

extern Variables gameVar;

#endif
