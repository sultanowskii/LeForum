#pragma once

#include <ncurses.h>

#include "lib/queue.h"
#include "lib/security.h"
#include "lib/status.h"

#include "client/state.h"

/**
 * @brief Structs that contains information about layout block 
 * 
 */
struct LeLayoutBlock {
	WINDOW   *win;           /* ncurses window */
	double    y_size_ratio;  /* how much space does this block take on axises */
	double    x_size_ratio; 
	int       y_size;        /* win size */
	int       x_size;
	int       y_coord;       /* win coordinates */
	int       x_coord;
	Queue    *states;        /* block states, to show different content */
	LeState  *current_state; /* pointer to the current state */
};
typedef struct LeLayoutBlock LeLayoutBlock;

status_t lelayoutblock_delete(LeLayoutBlock *lelayoutblock);