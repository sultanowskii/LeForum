#pragma ocnce

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <ncurses.h>

#include "lib/constants.h"
#include "lib/queue.h"
#include "lib/status.h"
#include "lib/util.h"

#include "client/state.h"
#include "client/layout.h"
#include "client/arg.h"

#define ctrl(ch) ((ch) & 0x1f)


/**
 * @brief LeLayoutBlock IDs
 * 
 */
enum LeLayoutBlockEnum {
	SIDEBAR,
	MAINCONTENT,
};

/**
 * @brief Color pairs for COLOR_PAIR(n)
 * 
 */
enum ColorPairs {
	PAIR_RED_BLACK = 1,
};

/**
 * @brief Sets the current lestate by its ID. 
 * 
 * @param id ID of desired state
 * @return LESTATUS_OK on success, LESTATUS_NFND if not found any state with the given ID 
 */
status_t           set_current_lestate(LeLayoutBlock *block, int id);

/**
 * @brief Updates layout block information. 
 * 
 */
void layout_update();

/**
 * @brief Handles user input for sidebar block (default state). 
 * 
 * @param ch Input key (from getch()) 
 */
void sidebar_default_handle(int ch);

/**
 * @brief Updates sidebar (default state). 
 * 
 */
void sidebar_default_update();

/**
 * @brief Updates main content (example state). 
 * 
 */
void main_content_example_update();

/**
 * @brief Handles user input for main content block (example state) 
 * 
 * @param ch Input key (from getch()) 
 */
void main_content_example_handle(int ch);

/**
 * @brief Initalises sidebar block. 
 * 
 * @return LESTATUS_OK on success 
 */
status_t sidebar_init();

/**
 * @brief Initalises main content block. 
 * 
 * @return LESTATUS_OK on success 
 */
status_t main_content_init();

/**
 * @brief Calls initialisators of blocks 
 * 
 * @return status_t 
 */
status_t lelayoutblocks_init();


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

status_t main(size_t argc, char **argv);