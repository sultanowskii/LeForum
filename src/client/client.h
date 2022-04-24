#pragma ocnce

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "lib/constants.h"
#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/status.h"
#include "lib/util.h"

#include "client/arg.h"

#define FILENAME_SERVERS ".leservers"

/**
 * @brief Main command list. 
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
 * @brief Server command list. 
 * 
 */
enum ServerCmdIDs {
	_scid_BEGIN,
	scid_CONNECT_DISCONNECT,
	scid_HISTORY,
	scid_BACK,
	_scid_END,
};

/**
 * @brief Thread command list. 
 * 
 */
enum ThreadCmdIDs {
	_tcid_BEGIN,
	tcid_FIND,
	tcid_INFO,
	tcid_MESSAGES,
	tcid_SEND_MESSAGE,
	tcid_BACK,
	_tcid_END,
};

/**
 * @brief Settings command list.  
 * 
 */
enum SettingsCmdIDs {
	_stgcid_BEGIN,
	stgcid_BACK,
	_stgcid_END,
};


struct ServerAddress {
	char      addr[32];
	uint16_t  port
};
typedef struct ServerAddress ServerAddress;

/* ---- Command string representators ----- */
/**
 * @brief Returns string representation of main command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *MainCmdID_REPR(enum MainCmdIDs id);

/**
 * @brief Returns string representation of server command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *ServerCmdID_REPR(enum ServerCmdIDs id);

/**
 * @brief Returns string representation of thread command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *ThreadCmdID_REPR(enum ThreadCmdIDs id);

/**
 * @brief Returns string representation of settings command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. LESTATUS_NFND is returned if there is no such command with given id
 */
const char *SettingsCmdID_REPR(enum SettingsCmdIDs id);
/* ---------------------------------------- */


/**
 * @brief Initialises program, 
 * 
 * @return LESTATUS_OK on success 
 */
status_t startup();

/**
 * @brief Cleans all the program data. 
 * 
 * @return LESTATUS_OK on success 
 */
status_t cleanup();

/**
 * @brief Program termination handler. Simply modifies g_working value. 
 * 
 * @param Signum
 */
void stop_program_handle(const int signum);

/**
 * @brief Inner connection to the server. 
 * 
 * @param addr Server addr
 * @param port Server port 
 * @return LESTATUS_OK on success. LESTATUS_CLIB if one of built-in functions failed. LESTATUS_IDAT if bad addr or port provdided 
 */
status_t __server_connect(const char *addr, uint16_t port);

/* ------------ Menu printers-------------- */
/**
 * @brief Prints server menu. 
 * 
 */
void print_menu_server();

/**
 * @brief Prints thread menu. 
 * 
 */
void print_menu_thread();

/**
 * @brief Prints settings menu. 
 * 
 */
void print_menu_settings();

/**
 * @brief Prints main menu. 
 * 
 */
void print_menu_main();
/* ---------------------------------------- */


/* ---------------- Prefix ---------------- */
/**
 * @brief Prints server prefix before user input
 * 
 */
void print_prefix_server();

/**
 * @brief Prints thread prefix before user input
 * 
 */
void print_prefix_thread();

/**
 * @brief Prints settings prefix before user input
 * 
 */
void print_prefix_settings();

/**
 * @brief Prints main prefix before user input
 * 
 */
void print_prefix_main();
/* ---------------------------------------- */


/* -------------- Commands ---------------- */
/**
 * @brief Processes Server commands. 
 * 
 */
void cmd_server();

/**
 * @brief Connects to the server
 * 
 */
void cmd_server_connect();

/**
 * @brief Disconnects from the current server
 * 
 */
void cmd_server_disconnect();

/**
 * @brief Prints server addresses history 
 * 
 */
void cmd_server_history();

/**
 * @brief Processes Thread commands.
 * 
 */
void cmd_thread();

/**
 * @brief Sends FIND query to the connected server with user input. 
 * Then it prints the result and lets user choose thread to be active. 
 * 
 */
void cmd_thread_find();

/**
 * @brief Shows active thread info. 
 * 
 */
void cmd_thread_info();

/**
 * @brief Sends GTHR query and prints message history of the active thread. 
 * 
 */
void cmd_thread_message_history();

/**
 * @brief Sends (aka posts) user message to the active thread. 
 * 
 */ 
void cmd_thread_send_message();

/**
 * @brief Processes Settings commands. 
 * 
 */
void cmd_settings();

/**
 * @brief Processes Exit commands. 
 * 
 */
void cmd_exit();
/* ---------------------------------------- */

/**
 * @brief Saves server addr in the file. 
 * 
 * @param addr Server addr 
 * @param port Server port  
 */
void save_server_addr(const char *addr, uint16_t port);

/**
 * @brief Loads server addr history from the file. 
 * 
 */
void load_server_addr_history();

/**
 * @brief Get the leclient file. Wrapper over fopen 
 * 
 * @param filename Filename 
 * @param mode Mode
 * @param create To create file if not found or not?
 * @return FD on success. LESTATUS_NSFD if file doesn't exist and create==FALSE 
 */
FILE * get_leclient_file(const char *filename, const char *mode, bool_t create);

/**
 * @brief Prints all the menues and stuff, then reads command from user. 
 * 
 * @param print_menu Function that prints menu
 * @param print_prefix Function that prints prefix before user input
 * @return User command
 */
int leclient_loop_process(void (*print_menu)(), void (*print_prefix)());

status_t main(size_t argc, char **argv);