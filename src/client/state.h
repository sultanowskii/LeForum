#pragma once

#include "lib/security.h"


/* Dependency */
struct LeLayoutBlock;
typedef struct LeLayoutBlock LeLayoutBlock;

/**
 * @brief IDs of Sidebar states
 * 
 */
enum SidebarStateIDs {
	_ssid_BEGIN,
	ssid_DEFAULT,
	_ssid_END,
};
const char * SidebarStateIDs_REPR(enum SidebarStateIDs id);

/**
 * @brief IDs of MainContent states
 * 
 */
enum MainContentStateIDs {
	_mcsid_BEGIN,
	mcsid_EXAMPLE,
	mcsid_SERVER,
	mcsid_THREADS,
	mcsid_THREAD,
	mcsid_SETTINGS,
	_mcsid_END,
};
const char * MainContentStateIDs_REPR(enum MainContentStateIDs id);

/**
 * @brief State of the LeLayoutBlock. One LeLayoutBlock may contain multiple states.
 * 
 */
struct LeState {
	int            id;
	const char    *name;
	void         (*update)();
	void         (*handle)(int ch);
	void          *data;
	void         (*data_destruct)(void *);
};
typedef struct LeState LeState;

/**
 * @brief Context data storage for Sidebar (no state, there is only one)
 * 
 */
struct SidebarData {

};
typedef struct SidebarData SidebarData;

/**
 * @brief Context data storage for MainContent (Example state)
 * 
 */
struct MainContentExampleData {
	char     *text;
};
typedef struct MainContentExampleData MainContentExampleData;

/**
 * @brief Destructs LeState object. Required by Queue 
 * 
 * @param lestate Pointer to LeState 
 * @return LESTATUS_OK on success 
 */
status_t           lestate_delete(LeState *lestate);

/**
 * @brief Destructs SiderbarData. Required by lestate_delete() 
 * 
 * @param data Pointer to SidebarData 
 * @return LESTATUS_OK on success 
 */
status_t           sidebardata_delete(SidebarData *data);

/**
 * @brief Destructs MainContentExampleData. Required by lestate_delete() 
 * 
 * @param data Pointer to SidebarData 
 * @return LESTATUS_OK on success 
 */
status_t           maincontentexampledata_delete(MainContentExampleData *data);