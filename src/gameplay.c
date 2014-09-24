#include "gameplay.h"

#ifdef DEBUG
	#include "log.h"
#endif


typedef enum _directions {NORTH, WEST, SOUTH, EAST} Directions;
typedef enum _deltas {NONE, N=-1, W=-1, S=1, E=1} Deltas;

typedef void (*action)(Board* const, Board* const);

static Board * getDelta(Board * const * const board, const int y, const int x);
static void checkTarget(Board * const * const board, const int y, const int x, action getAction);
static void checkSoldierRadius(Board * const * const board, const int y, const int x);
static int canShoot(Board * const * const board, const int x, const int y);
static void getActionInf(Board * const infected, Board * const target);
static void getActionDoc(Board * const doctor, Board * const target);
static void getActionCit(Board * const citizen, Board * const target);
static void getActionSol(Board * const * const board, Board * const soldier, Board * const target, const int y, const int x);
static void getActionNurse(Board * const nurse, Board * const target);
static int checkOutBounds(const Directions move, const int y, const int x);

/*
 * getMoves(Board * const * const board) moves the characters who can move.
 *
 * It takes in as a parameter the board array. It then iterates over it, gets a
 * random direction, checks if the character can move then moves it.
 *
 * Remark: getMoves() needs to be called by casting the two lists with 
 * (Board * const * const) to squash a harmless (in this case) warning
 * from GCC.
 * See http://c-faq.com/ansi/constmismatch.html
 */
void getMoves(Board * const * const board)
{
	for (size_t i = 0; i < gameVar.dim.y; i++) {
		for (size_t j = 0; j < gameVar.dim.x; j++) {
			switch (board[i][j].character) {
				case INF:
				case DOC:
				case CIT:
				case SOL:
				case NUR:
					board[i][j].direction = rand()%4;
					if (checkOutBounds(board[i][j].direction, i, j)) {
						Board *target = getDelta(board, i, j);
						if ((board[i][j].character == CIT && target->character == INF)
							|| (board[i][j].character == INF && target->character == CIT)) {
							break;
						} else if (target->character != WALL && target->character != DEAD) {
							Characters tmp = target->character;
							target->character = board[i][j].character;
							board[i][j].character = tmp;
						}
					}
					break;
				case DEAD:	//fallthrough
				case WALL:
				case EMPTY:
					break;
			}
		}
	}
}

/*
 * getActions(Board * const * const board) checks if the unit can execute his action.
 *
 * It takes in as a parameter the board array. It then iterates over it, gets a
 * random direction, checks if the character can do his action then do it.
 *
 * Remark: getActions() needs to be called by casting the two lists with 
 * (Board * const * const) to squash a harmless (in this case) warning
 * from GCC.
 * See http://c-faq.com/ansi/constmismatch.html
 */
void getActions(Board * const * const board) 
{
	Units *units = &gameVar.units;
	Time *times = &gameVar.time;

	for (size_t i = 0; i < gameVar.dim.y; i++) {
		for (size_t j = 0; j < gameVar.dim.x; j++) {
			++times->elapsed;
			board[i][j].direction = rand()%4; 
			switch (board[i][j].character) {
				case WALL:	//fallthrough
				case EMPTY:
					break;
				case DEAD:
					if (rand()%100 == 1) {
						board[i][j].character = EMPTY;
						--units->dead;
					}
				case INF:
					for (board[i][j].direction = 0; board[i][j].direction < 4; board[i][j].direction++) {
						if (checkOutBounds(board[i][j].direction, i, j)) { 
							checkTarget(board, i, j, getActionInf);
						}
					}
					break;
				case DOC:
					if (checkOutBounds(board[i][j].direction, i, j)) {
						checkTarget(board, i, j, getActionDoc);	
					}
					break;
				case CIT:
					if (rand()%100 >= 98 && checkOutBounds(board[i][j].direction, i, j)) {
						checkTarget(board, i, j, getActionCit);
					}
					break;
				case SOL:
					checkSoldierRadius(board, i, j);
					break;
				case NUR:
					if (checkOutBounds(board[i][j].direction, i, j)) {
						checkTarget(board, i, j, getActionNurse);
					}
			}
		}
	}
}

/*
 * getDelta(Board * const * const board, const int y, const int x) finds the target according to the direction of the action.
 *
 * It takes in as parameters the board array and the original coordinates. 
 * According to the direction of the action of the original actioner, it finds
 * the target and returns it.
 */
Board * getDelta(Board * const * const board, const int y, const int x)
{
	Board *attacker = &board[y][x];
	Board *target = malloc(sizeof(Board));	
	Deltas delta = attacker->direction == NORTH ? N : attacker->direction == SOUTH ? S : attacker->direction == WEST ? W : E;

	if (attacker->direction == NORTH || attacker->direction == SOUTH) {
		target = &board[y + delta][x];
	} else {
		target = &board[y][x + delta];
	}

	return target;

} 

/*
 * checkTarget(Board * const * const board, const int y, const int x, action getAction) finds the target and gets the action.
 *
 * It takes in as parameters the board array, the original coordinates and a
 * function pointer to action to execute. After getting the target through
 * getDelta(), it executes the action using getAction();
 */
void checkTarget(Board * const * const board, const int y, const int x, action getAction)
{
	Board *target = NULL;
	
	target = getDelta(board, y, x);

	if (target == NULL) exit(EXIT_FAILURE);

	getAction(&board[y][x], target);
}

void checkSoldierRadius(Board * const * const board, const int y, const int x)
{
	int r1 = rand()%2, r2 = rand()%3;
	int xt = x, yt = y;

	if (rand()%2) {
		yt += r1;
	} else {
		yt -= r1;
	}

	if (rand()%2) {
		xt =+ r2;
	} else {
		xt -= r2;
	}

	if (xt >= 0 && xt < gameVar.dim.x && yt >=0 && yt < gameVar.dim.y) {
		Board *attacker = &board[y][x];
		Board *target = &board[yt][xt];
		getActionSol(board, attacker, target, y, x);
	}
}

void getActionInf(Board * const infected, Board * const target)
{
	Units *units = &gameVar.units;
	Time *times = &gameVar.time;

	int prob = rand()%100;

	if (rand()%100 >= 95) {
		if (target->character == CIT && prob >= 75) {
			--units->citizens;
			++units->infected;

			target->character = INF;
			times->elapsed = 0;
			#ifdef DEBUG
				logMsg("Inf converts Cit to Inf\n");
			#endif
		} else if (target->character == WALL && prob < 5) {
			target->character = EMPTY;
			units->wood += (rand()%25)+1;
			#ifdef DEBUG
				logMsg("Inf destroys Wall\n");
			#endif
		} else if (target->character == DOC || target->character == NUR) {
			if (prob < 25) {
				if (target->character == DOC) {
					--units->doctors;
					#ifdef DEBUG
						logMsg("Inf kills Doc\n");
					#endif
				} else {
					--units->nurses;
					#ifdef DEBUG
						logMsg("Inf kills Nur\n");
					#endif
				}
				++units->dead;

				target->character = DEAD;
			} else if (prob < 50) {
				--units->infected;
				++units->dead;

				infected->character = DEAD;
				#ifdef DEBUG
					logMsg("Inf killed by Doc/Nur\n");
				#endif
			} else if (prob < 75) {
				if (target->character == DOC) {
					--units->doctors;
					#ifdef DEBUG
						logMsg("Inf converts Doc to Inf\n");
					#endif
				} else {
					--units->nurses;
					#ifdef DEBUG
						logMsg("Inf converts Nur to Inf\n");
					#endif
				}
				++units->infected;

				target->character = INF;
			} else if (prob < 90) {
				--units->infected;
				++units->citizens;

				infected->character = CIT;
				#ifdef DEBUG
					logMsg("Inf cured to Cit\n");
				#endif
			} else if (prob < 100) {
				if (target->character == DOC) {
					--units->doctors;
					#ifdef DEBUG
						logMsg("Inf traded with Doc\n");
					#endif
				} else {
					--units->nurses;
					#ifdef DEBUG
						logMsg("Inf traded with Nur\n");
					#endif
				}
				--units->infected;
				units->dead += 2;

				infected->character = DEAD;
				target->character = DEAD;
			}
		} else if (target->character == SOL) {
			if (prob < 10) {
				--units->infected;
				++units->dead;

				infected->character = DEAD;
				#ifdef DEBUG
					logMsg("Inf killed by Soldier\n");
				#endif
			} else if (prob < 60) {
				--units->soldiers;
				++units->dead;

				target->character = INF;
				infected->character = DEAD;
				#ifdef DEBUG
					logMsg("Inf dies against Sol but converts him\n");
				#endif
			} else if (prob < 90) {
				--units->soldiers;
				++units->infected;

				target->character = INF;
				#ifdef DEBUG
					logMsg("Inf converts Sol to Inf\n");
				#endif
			} else {
				--units->soldiers;
				++units->dead;

				target->character = DEAD;
				#ifdef DEBUG
					logMsg("Inf kills Sol\n");
				#endif
			}
		}
		times->elapsed = 0;
	}
}

void getActionDoc(Board * const doctor, Board * const target)
{
	Units *units = &gameVar.units;
	Time *times = &gameVar.time;

	int prob = rand()%100;
	if (prob < 10) {
		if (target->character == CIT) {
			--units->citizens;
			++units->nurses;

			target->character = NUR;
			#ifdef DEBUG
				logMsg("Doc converts Cit to Nur\n");
			#endif
		} else if (target->character == NUR) {
			--units->nurses;
			++units->doctors;

			target->character = DOC;
			#ifdef DEBUG
				logMsg("Doc converts Nur to Doc\n");
			#endif
		}
	} else if (target->character == INF) {
		if (prob < 25) {
			++units->citizens;
			--units->infected;

			target->character = CIT;
			#ifdef DEBUG
				logMsg("Doc converts Inf to Cit\n");
			#endif
		}
	} else if (target->character == DEAD) {
		if (prob == 10) {
			++units->citizens;
			--units->dead;

			target->character = CIT;
			#ifdef DEBUG
				logMsg("Doc revives Dead\n");
			#endif
		}
	}

	times->elapsed = 0;
}

void getActionCit(Board * const citizen, Board * const target)
{
	Units *units = &gameVar.units;
	Time *times = &gameVar.time;

	int prob = rand()%100;
	if (prob == 20) {
		--units->citizens;
		++units->soldiers;

		citizen->character = SOL;
		#ifdef DEBUG
			logMsg("Cit converts to Sol\n");
		#endif
	} else if (prob < 15) {
		if (target->character == EMPTY && times->days >= 100 && units->wood >= 25) {
			target->character = WALL;
			units->wood -= (rand()%25)+1;
			times->elapsed = 0;
			#ifdef DEBUG
				logMsg("Cit builds a wall\n");
			#endif
		}
	}
}

void getActionSol(Board * const * const board, Board *soldier, Board *target, const int y, const int x)
{
	Units *units = &gameVar.units;
	Time *times = &gameVar.time;

	int prob = rand()%100;
	if (prob < 2 && (target->character == DOC || target->character == NUR)) {
		prob = rand()%100;
	} 

	if (prob < 80) { 
		if (canShoot(board, y, x)) {
			if (target->character == INF) {
				++units->dead;
				--units->infected;

				target->character = DEAD;
				times->elapsed = 0;
				#ifdef DEBUG
					logMsg("Sol kills Inf\n");
				#endif
			} else if (prob < 2 && (target->character == NUR || target->character == DOC)) {
				if (target->character == NUR) {
					--units->nurses;
					#ifdef DEBUG
						logMsg("Sol kills Nur\n");
					#endif
				} else {
					--units->doctors;
					#ifdef DEBUG
						logMsg("Sol kills Doc\n");
					#endif
				}
				++units->dead;

				target->character = DEAD;
				times->elapsed = 0;
			}
		}

		if (target->character == CIT) {
			if (prob >= 80) {
				--units->citizens;
				++units->soldiers;

				target->character = SOL;
				times->elapsed = 0;
				#ifdef DEBUG
					logMsg("Sol converts Cit\n");
				#endif
			}
		} else if (target->character == DEAD) {
			--units->dead;

			target->character = EMPTY;
			#ifdef DEBUG
				logMsg("Sol cleans Dead\n");
			#endif
		}
	}
}

int canShoot(Board * const * const board, const int y, const int x)
{
	for (int i = y - 2; i < y + 3; i++) {
		for (int j = x - 3; j < x + 4; j++) {
			if (j >= 0 && j < gameVar.dim.x && i >=0 && i < gameVar.dim.y) {
				if (board[i][j].character == CIT) {
					return 0;
				}
			}
		}
	}

	return 1;
}

void getActionNurse(Board * const nurse, Board * const target)
{
	Units *units = &gameVar.units;

	int prob = rand()%100;
	if (target->character == INF) {
		/*if (prob < 3) {
			--countInf;
			++countNur;

			target->character = NUR;
		} else*/ if (prob < 20) {
			--units->infected;
			++units->citizens;

			target->character = CIT;
			#ifdef DEBUG
				logMsg("Nur heals Inf\n");
			#endif
		}
	}
}

int checkOutBounds(const const Directions move, const const int y, const const int x)
{
	if (move == NORTH) {		
		return (y + N >= 0);
	} else if (move == SOUTH) {
		return (y + S < gameVar.dim.y);
	} else if (move == WEST) {
		return (x + W >= 0);
	} else {
		return (x + E < gameVar.dim.x);
	}
}
