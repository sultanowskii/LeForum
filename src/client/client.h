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

/**
 * @brief LeLayoutBlock IDs
 * 
 */
enum LeLayoutBlockEnum {
	SIDEBAR,
	MAINCONTENT,
};

enum ColorParis {
	PAIR_RED_BLACK = 1,
};

/**
 * @brief Updates layout block information. 
 * 
 * @param sidebar Pointer to sidebar block
 * @param main_content Pointer to main content block
 */
void layout_update(LeLayoutBlock *sidebar, LeLayoutBlock *main_content);

/**
 * @brief Updates sidebar.
 * 
 * @param sidebar Pointer to sidebar block 
 */
void sidebar_update(LeLayoutBlock *sidebar);

/**
 * @brief Updates main content (example state). 
 * 
 * @param main_content Pointer to main content block 
 */
void main_content_example_update(LeLayoutBlock *main_content);

/**
 * @brief Handles user input for main content block 
 * 
 * @param main_content Pointer to main content block 
 * @param ch Input key (from getch()) 
 */
void main_content_handle(LeLayoutBlock *main_content, int ch);

/**
 * @brief Initalises sidebar block. 
 * 
 * @param sidebar Pointer to pointer to sidebar block 
 * @return LESTATUS_OK on success 
 */
status_t sidebar_init(LeLayoutBlock **sidebar);

/**
 * @brief Initalises main content block. 
 * 
 * @param sidebar Pointer to pointer to main content block 
 * @return LESTATUS_OK on success 
 */
status_t main_content_init(LeLayoutBlock **main_content);

/**
 * @brief Calls initialisators of blocks 
 * 
 * @param sidebar 
 * @param main_content 
 * @return status_t 
 */
status_t lelayoutblocks_init(LeLayoutBlock **sidebar, LeLayoutBlock **main_content);


/**
 * @brief Initialises program 
 * 
 * @param sidebar Pointer to pointer to sidebar block 
 * @param main_content Pointer to pointer to main content block 
 * @return LESTATUS_OK on success 
 */
status_t startup(LeLayoutBlock **sidebar, LeLayoutBlock **main_content);

/**
 * @brief Cleans all the program data 
 * 
 * @param sidebar Pointer to pointer to sidebar block 
 * @param main_content Pointer to pointer to main content block 
 * @return LESTATUS_OK on success 
 */
status_t cleanup(LeLayoutBlock **sidebar, LeLayoutBlock **main_content);

/**
 * @brief Program termination handler. Simply modifies g_working value. 
 * 
 * @param Singnum
 */
void stop_program_handle(const int signum);

status_t main(size_t argc, char **argv);