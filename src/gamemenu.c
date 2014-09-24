#include <menu.h>
#include <string.h>
#include "gamemenu.h"
#include "list.h"
#include "display.h"
//defines
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define ENTER 	10
//constants
enum {DOWN = -1, UP = 1};
enum {QUIT, PLAY};
enum {XC, YC, DOCC, INFC, NURC, SOLC, WOODC};

//Arrays containing the menu item names and descriptions

static char *choices[] = {     
    "Game width: ",
    "Game height: ",
    "Number of doctors: ",
    "Number of infected: ",
    "Number of nurses: ",
    "Number of soldiers: ",
    "Amount of wood: ",
    "Speed: ",
    "Step-by-step: ",
    "Recalculate units",
    "Play",
    
    (char *)NULL
};

static char *speeds[] = {
    "Slow",
    "Fast",
    "Fastest",

    (char *)NULL
};

static char *steps[] = {
    "Yes",
    "No",
    (char *)NULL
};

//internal global vars
static ITEM **my_items = NULL;
static MENU *my_menu = NULL;
static WINDOW *my_menu_win = NULL;

//internal func prototypes
static int eventLoop(const List ** const listSpeeds, const List ** const listSteps, int counters[]);
static void quitMenu(List *listSpeeds, List *listSteps);
static void fillItems(const int counters[], const List * const listSpeeds, const List * const listSteps);
static char * convertToHeapString(const char *string);
static void updateUnits(int counters[]);
static void updateUnitsDisplay(int counters[]);
static void updateGameVariables(const int counters[], const List * const listSpeeds, const List * const listSteps);
static void set_item_description (ITEM *item, const char *description);
static void toggleValue(ITEM *item, const int direction, const List ** const listSpeeds, const List ** const listSteps, int counters[]);
static void updateNumericValue(ITEM *item, const int direction, int counters[], const int index);

void displayMenu()
{	
    int n_choices = 0;

	List *listSpeeds = createCircularLinkedList((const char * const *)speeds);
	listSpeeds = listSpeeds->previous; // Default value is Faster
	List *listSteps = createCircularLinkedList((const char * const *)steps);
	listSteps = listSteps->next; // Default value is No

	int counters[7];
	counters[XC] = 50;
	counters[YC] = 50;
	updateUnits(counters);

	/* Initialize items */
	n_choices = ARRAY_SIZE(choices);
	my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
	fillItems(counters, listSpeeds, listSteps);

	/* Initialize curses */
	initNcurses();

	/* Get offsets */
	int offsetx = (COLS - 40) / 2;
        int offsety = 0;

	/* Create menu */
	my_menu = new_menu((ITEM **)my_items);

	/* Create the window to be associated with the menu */
	my_menu_win = newwin(15, 40, offsety, offsetx);
	keypad(my_menu_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu, my_menu_win);
	set_menu_sub(my_menu, derwin(my_menu_win, 12, 38, 3, 1));
	set_menu_format(my_menu, 16, 1);

	/* Set menu mark */
	set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
	box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, 40, "Infect", COLOR_PAIR(1));
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(my_menu_win, 2, 39, ACS_RTEE);

	/* Set foreground and background of the menu */
	set_menu_fore(my_menu, COLOR_PAIR(1) | A_REVERSE);
	set_menu_back(my_menu, COLOR_PAIR(2));
	set_menu_grey(my_menu, COLOR_PAIR(4));

	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);

	mvprintw(LINES - 3, 0, "Up and down arrow keys to navigate");
	mvprintw(LINES - 2, 0, "Left and Right to increment and decrement");
	mvprintw(LINES - 1, 0, "Enter to select, q to quit");
	refresh();

	if (eventLoop((const List ** const)&listSpeeds, (const List ** const)&listSteps, counters) == PLAY) {
		updateGameVariables(counters, listSpeeds, listSteps);
		quitMenu(listSpeeds, listSteps);
	} else {
		quitMenu(listSpeeds, listSteps);
		exit(EXIT_SUCCESS);
	}
}

/*
 * eventLoop(const List ** const listSpeeds, const List **const listSteps, int counters[]) calls other functions depending on user requests.
 *
 * It takes in as parameters the data structures containing the items values
 * /descriptions but does not modify them directly. It's a switchboard.
 *
 * It return a boolean value depending on the user's desire to quit or play the
 * game.
 *
 * Remark: eventLoop() needs to be called by casting the two lists with 
 * (const Board ** const) to squash a harmless (in this case) warning
 * from GCC.
 * See http://c-faq.com/ansi/constmismatch.html
 */
int eventLoop(const List ** const listSpeeds, const List **const listSteps, int counters[])
{
	int c = 0;
	ITEM *cur_item = NULL;

	while((c = getch()) != 'q')
	{       
		switch(c) {
			case KEY_LEFT: {
				cur_item = current_item(my_menu);
				if (item_index(cur_item) < 9) {
					toggleValue(cur_item, DOWN, listSpeeds, listSteps, counters);
				}
				//	This refreshes the item description on the screen
				// 	Not sure how to do it otherwise
				menu_driver(my_menu, REQ_DOWN_ITEM);
				menu_driver(my_menu, REQ_UP_ITEM);
			}
				break;

			case KEY_RIGHT: {	
				cur_item = current_item(my_menu);
				if (item_index(cur_item) < 9) {
					toggleValue(cur_item, UP, listSpeeds, listSteps, counters);
				}
				menu_driver(my_menu, REQ_DOWN_ITEM);
				menu_driver(my_menu, REQ_UP_ITEM);
			}
				break;

			case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
				break;

			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
				break;

			case ENTER: {
				int index = item_index(current_item(my_menu));
				if (index == 9) {
					updateUnitsDisplay(counters);
					//	Iterate over every menu item to refresh their displayed value
					for (size_t i = 0; i < 10; i++) {
						menu_driver(my_menu, REQ_UP_ITEM);
					}
				} else if (index == 10) {
					return PLAY;
				}
			}
		} // end switch

		wrefresh(my_menu_win);
	} // end while

	return QUIT;
}

/*
 * quitMenu(List *listSpeeds, List *listSteps) is the exit point of this module.
 *
 * It frees all the memory allocated in displayMenu() and destroys our menu
 * before returning to the command line.
 */
void quitMenu(List *listSpeeds, List *listSteps)
{
	unpost_menu(my_menu);
	freeCircularLinkedList(&listSpeeds);
	freeCircularLinkedList(&listSteps);
	int n_choices = ARRAY_SIZE(choices);
	for(size_t i = 0; i < n_choices; ++i)
		free_item(my_items[i]);
	free_menu(my_menu);
	endwin();
}

/*
 * fillItems(const int counters[], const List * const listSpeeds, const List * const listSteps) initialises our menu items.
 *
 * It takes in as parameters the data structures containing the items possible
 * descriptions/values, then fills in the item's value using these.
 */
void fillItems(const int counters[], const List * const listSpeeds, const List * const listSteps)
{
	/* 	
	 *	You should only pass strings from the heap, global memory or
	 *	the string literal pool to new_item(). Why? Because if you 
	 *	look through the source, the item description points to the
	 *	string we passed, so you shouldn't pass strings from the stack.
	 *	Freeing the description before changing it is left to the user.
	 *
	 *	This results in some ugly things, not only to convert counters 
	 *	to strings, but also to make sure we can free the strings safely 
	 *	later on. This is the best way to do this I've found so far.
	 */

	for (size_t i = XC; i <= WOODC; i++) {
		char buffer[10];
		sprintf(buffer, "%d", counters[i]);
		my_items[i] = new_item(choices[i], convertToHeapString(buffer));
	}

	my_items[7] = new_item(choices[7], convertToHeapString(listSpeeds->value));
	my_items[8] = new_item(choices[8], convertToHeapString(listSteps->value));
	my_items[9] = new_item(choices[9], NULL);
	my_items[10] = new_item(choices[10], NULL);
}

/*
 * convertToHeapString(const char *string) takes a string and duplicates it on the heap.
 *
 * It takes in as parameter a stack string and return a heap string. Freeing
 * the string is left to the caller.
 */
char * convertToHeapString(const char *string)
{
	char *heapString = strdup(string);
	if (heapString == NULL) {
		printError("Could not convert to heap string.\n");
	}

	return heapString;
}

/*
 * updateUnits(int counters[]) recalculates unit numbers according to the board dimensions.
 *
 * It takes in as parameters the array containing our unit counters.
 */
void updateUnits(int counters[])
{
	counters[DOCC] = (counters[XC] * counters[YC] * 0.01) + 1;
	counters[INFC] = (counters[XC] * counters[YC] * 0.005) + 1;
	counters[SOLC] = (counters[XC] * counters[YC] * 0.02) + 1;
	counters[NURC] = (counters[XC] * counters[YC] * 0.05) + 1;
	counters[WOODC] = (counters[XC] * counters[YC] * 0.5);
}

/*
 * updateUnitsDisplay(int counters[]) recalculates unit numbers and refreshes them on screen.
 *
 * It takes in as parameters the array containing our unit counters.
 */
void updateUnitsDisplay(int counters[])
{
	char buffer[10];
	updateUnits(counters);

	for (size_t i = 0; i <= WOODC; i++) {
		sprintf(buffer, "%d", counters[i]);
		set_item_description(my_items[i], convertToHeapString(buffer));
	}
}

/*
 * updateGameVariables(const int counters[], const List * const listSpeeds, const List * const listSteps) updates the global game variables before exiting the menu.
 *
 * It takes in as parameters the data structures containing the items values
 * /descriptions and updates the game variables using these.
 */
 void updateGameVariables(const int counters[], const List * const listSpeeds, const List * const listSteps)
{
	Units *units = &gameVar.units;
	Time *times = &gameVar.time;

	gameVar.dim.x = counters[XC];
	gameVar.dim.y = counters[YC];

	units->doctors = counters[DOCC];
	units->infected = counters[INFC];
	units->nurses = counters[NURC];
	units->soldiers = counters[SOLC];
	units->wood = counters[WOODC];
	
	if (strcmp(listSpeeds->value, "Slow") == 0) {
		times->refreshRate = SLOW;
	} else if (strcmp(listSpeeds->value, "Fast") == 0) {
		times->refreshRate = FAST;
	} else {
		times->refreshRate = FASTEST;
	}

	times->steps = strcmp(listSteps->value, "Yes") == 0 ? 1 : 0;
}

/*
 * set_item_description(ITEM *item, const char *description) changes an item's description.
 *
 * It takes in as parameters the item and the wanted description.
 * The previous description is freed then pointed to the new one.
 */
void set_item_description (ITEM *item, const char *description)
{   
	if (item->description.str != NULL) {
		free((void*)(item->description).str);
	}

	item->description.length = strlen(description);
	item->description.str = description;
}

/*
 * toggleValue(ITEM *item, const int direction, const List ** const listSpeeds, const List ** const listSteps, int counters[]) toggles the value or in/decrements it indirectly.
 *
 * It takes in as parameters the data structures containing the items values
 * /descriptions, the toggling direction and the relevant item. Depending on
 * the direction, the value of the data structure is changed and this new value
 * is used to update the item's description.
 */
void toggleValue(ITEM *item, const int direction, const List ** const listSpeeds, const List ** const listSteps, int counters[])
{
	int index = item_index(item);

	if (index == 7) {
		*listSpeeds = direction == UP ? (*listSpeeds)->next : (*listSpeeds)->previous;
		set_item_description(item, convertToHeapString((*listSpeeds)->value));
	} else if (index == 8) {
		*listSteps = direction == UP ? (*listSteps)->next : (*listSteps)->previous;
		set_item_description(item, convertToHeapString((*listSteps)->value));
	} else {
		updateNumericValue(item, direction, counters, index);
	}
}

/*
 * updateNumericValue(ITEM *item, const int direction, int counters[], const int index) in/decrements the item's value if it's a number.
 *
 * It takes in as parameters the array containing the items values, the toggling
 * direction and the relevant item with its index in the array. Depending on
 * the direction, the value in the array is incremented or decremented and this 
 * new value is used to update the item's description.
 */
void updateNumericValue(ITEM *item, const int direction, int counters[], const int index)
{
	char buffer[10];

	if (direction == UP) {
		sprintf(buffer, "%d", ++counters[index]);
		set_item_description(item, convertToHeapString(buffer));
	} else {
		if (counters[index] == 0) {
			return;
		} else {
			sprintf(buffer, "%d", --counters[index]);
			set_item_description(item, convertToHeapString(buffer));
		}
	}
}
