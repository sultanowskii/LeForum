#include "client/state.h"

status_t lestate_delete(LeState *lestate) {
	NULLPTR_PREVENT(lestate, LESTATUS_NPTR)

	lestate->data_destruct(lestate->data);
	free(lestate);
	
	return LESTATUS_OK;
}

status_t sidebardata_delete(SidebarData *data) {
	NULLPTR_PREVENT(data, LESTATUS_NPTR)

	free(data);
	
	return LESTATUS_OK;
}

status_t maincontextexampledata_delete(MainContentExampleData *data) {
	NULLPTR_PREVENT(data, LESTATUS_NPTR)

	if (data->text != nullptr) {
		free(data->text);
	}
	free(data);
	
	return LESTATUS_OK;
}