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
	_mcid_END
};

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
 * @brief Prints main menu 
 */
void print_main_menu();

status_t main(size_t argc, char **argv);