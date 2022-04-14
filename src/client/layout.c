#include "client/layout.h"

status_t lelayoutblock_delete(LeLayoutBlock *lelayoutblock) {
	NULLPTR_PREVENT(lelayoutblock, LESTATUS_NPTR)

	if (lelayoutblock->win != nullptr) {
		delwin(lelayoutblock->win);
		lelayoutblock->win = nullptr;
	}

	if (lelayoutblock->states != nullptr) {
		queue_delete(lelayoutblock->states);
		lelayoutblock->states = nullptr;
	}

	lelayoutblock->current_state = nullptr;
	
	free(lelayoutblock);

	return LESTATUS_OK;
}