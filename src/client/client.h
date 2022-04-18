#pragma ocnce

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "lib/constants.h"
#include "lib/queue.h"
#include "lib/status.h"
#include "lib/util.h"

#include "client/arg.h"

/**
 * @brief Main command list 
 * 
 */
enum MainCmdIDs {
	_mcid_BEGIN,
	mcid_SERVER,
	mcid_THREAD,
	mcid_SETTINGS,
	mcid_EXIT,
	_mcid_END,
};

/**
 * @brief Server command list 
 * 
 */
enum ServerCmdIDs {
	_scid_BEGIN,
	scid_CONNECT_DISCONNECT,
	scid_HISTORY,
	_scid_END,
};

/**
 * @brief Thread command list 
 * 
 */
enum ThreadCmdIDs {
	_tcid_BEGIN,
	tcid_INFO,
	tcid_MESSAGES,
	tcid_POST_MESSAGE,
	_tcid_END,
};

/**
 * @brief Settings command list 
 * 
 */
enum SettingsCmdIDs {
	_stgcid_BEGIN,
	_stgcid_END,
};

/**
 * @brief Returns string representation of main command with provided id
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *MainCmdID_REPR(enum MainCmdIDs id);

/**
 * @brief Returns string representation of server command with provided id
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *ServerCmdID_REPR(enum ServerCmdIDs id);

/**
 * @brief Returns string representation of thread command with provided id
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *ThreadCmdID_REPR(enum ThreadCmdIDs id);

/**
 * @brief Returns string representation of settings command with provided id
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *SettingsCmdID_REPR(enum SettingsCmdIDs id);


/**
 * @brief Initialises program 
 * 
 * @return LESTATUS_OK on success 
 */
status_t startup();

/**
 * @brief Cleans all the program data 
 * 
 * @return LESTATUS_OK on success 
 */
status_t cleanup();

/**
 * @brief Program termination handler. Simply modifies g_working value. 
 * 
 * @param Singnum
 */
void stop_program_handle(const int signum);

/**
 * @brief Prints server menu
 * 
 */
void print_server_menu();

/**
 * @brief Prints thread menu
 * 
 */
void print_thread_menu();

/**
 * @brief Prints settings menu
 * 
 */
void print_settings_menu();

/**
 * @brief Prints main menu 
 * 
 */
void print_main_menu();

status_t main(size_t argc, char **argv);