#include "client/state.h"

inline const char * MainContentStateIDs_REPR(enum MainContentStateIDs id) {
	switch (id) {
		case mcsid_EXAMPLE:       return "Example";
		case mcsid_SERVER:        return "Servers";
		case mcsid_THREADS:       return "Threads";
		case mcsid_THREAD:        return "Thread view";
		case mcsid_SETTINGS:      return "Settings";
		default:                  return nullptr;
	}
}

inline const char * SidebarStateIDs_REPR(enum SidebarStateIDs id) {
	switch (id) {
		case ssid_DEFAULT:   return "Default";
		default:             return nullptr;
	}
}

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

status_t maincontentexampledata_delete(MainContentExampleData *data) {
	NULLPTR_PREVENT(data, LESTATUS_NPTR)

	if (data->text != nullptr) {
		free(data->text);
	}
	free(data);
	
	return LESTATUS_OK;
}