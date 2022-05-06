#ifndef LEFORUM_CLIENT_CLIENT_H_
#define LEFORUM_CLIENT_CLIENT_H_

#include <stdint.h>
#include <stdio.h>

#include "client/query.h"
#include "lib/constants.h"
#include "lib/status.h"
#include "lib/util.h"

#define FILE_SERVER_HISTORY  "servers"
#define DIR_CLIENT           ".lefourm/"

/**
 * @brief Main command list. 
 * 
 */
enum MainCmdIDs {
	_MCID_BEGIN,
	MCID_SERVER,
	MCID_THREAD,
	MCID_SETTINGS,
	MCID_EXIT,
	_MCID_END,
};

/**
 * @brief Server command list. 
 * 
 */
enum ServerCmdIDs {
	_SCID_BEGIN,
	SCID_CONNECT_DISCONNECT,
	SCID_INFO,
	SCID_HISTORY,
	SCID_BACK,
	_SCID_END,
};

/**
 * @brief Thread command list. 
 * 
 */
enum ThreadCmdIDs {
	_TCID_BEGIN,
	TCID_CREATE,
	TCID_FIND,
	TCID_INFO,
	TCID_MESSAGES,
	TCID_SEND_MESSAGE,
	TCID_BACK,
	_TCID_END,
};

/**
 * @brief Settings command list. 
 * 
 */
enum SettingsCmdIDs {
	_STGCID_BEGIN,
	STGCID_BACK,
	_STGCID_END,
};

/**
 * @brief Inner connection to the server. 
 * 
 * @param addr Server addr
 * @param port Server port 
 * @return LESTATUS_OK on success. LESTATUS_CLIB if one of built-in functions failed. 
 * LESTATUS_IDAT if bad addr or port provdided. LESTATUS_EXST if already connected to some server
 */
status_t __server_connect(const char *addr, uint16_t port);

/**
 * @brief Disconnects from the current server. 
 * 
 * @return LESTATUS_OK on success. LESTATUS_NFND if not connected to server 
 */
status_t __server_disconnect();

/* ---- Command string representators ----- */
/**
 * @brief Returns string representation of main command with provided id. 
 * 
 * @param id Command ID 
 * @return Command string representation. 
 * LESTATUS_NFND is returned if there is no such command with given id 
 */
const char *MainCmdID_REPR(enum MainCmdIDs id);

/**
 * @brief Returns string representation of server command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. 
 * LESTATUS_NFND is returned if there is no such command with given id 
 */
const char *ServerCmdID_REPR(enum ServerCmdIDs id);

/**
 * @brief Returns string representation of thread command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. 
 * LESTATUS_NFND is returned if there is no such command with given id
 */
const char *ThreadCmdID_REPR(enum ThreadCmdIDs id);

/**
 * @brief Returns string representation of settings command with provided id. 
 * 
 * @param id Command ID
 * @return Command string representation. 
 * LESTATUS_NFND is returned if there is no such command with given id
 */
const char *SettingsCmdID_REPR(enum SettingsCmdIDs id);
/* ---------------------------------------- */


/* ------------ File stuff----------------- */
/**
 * @brief Get the leclient file. Wrapper over fopen. 
 * 
 * @param filename Filename 
 * @param mode Mode
 * @param create To create file if not found or not?
 * @return FD on success. LESTATUS_NSFD if file doesn't exist and create==FALSE 
 */
FILE *get_leclient_file(const char *filename, const char *mode, bool_t create);

/**
 * @brief Saves server addr in the file. 
 * 
 * @param addr Server addr 
 * @param port Server port 
 * @return LESTATUS_OK on success 
 */
status_t server_addr_save(const char *addr, uint16_t port);

/**
 * @brief Loads server addr history from the file. 
 * 
 * @return LESTATUS_OK on success 
 */
status_t server_addr_history_load();

/**
 * @brief Saves token of the current server in the file. 
 * 
 * @param token Token to save
 * @return LESTATUS_OK on success. LESTATUS_IDAT no thread is active 
 */
status_t token_save(char *token);

/**
 * @brief Loads token (if exists) from the file. 
 * 
 * @return Token on success. 
 * LESTATUS_NSFD if not found. LESTATUS_IDAT no thread is active 
 */
char *token_load();
/* ---------------------------------------- */


/* ------------ Menu printers-------------- */
/**
 * @brief Prints all the menues and stuff, then reads command from user. 
 * 
 * @param print_menu Function that prints menu 
 * @param print_prefix Function that prints prefix before user input 
 * @return User command 
 */
int leclient_loop_process(void (*print_menu)(), void (*print_prefix)());

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
 * @brief Prints server prefix before user input, 
 * 
 */
void print_prefix_server();

/**
 * @brief Prints thread prefix before user input. 
 * 
 */
void print_prefix_thread();

/**
 * @brief Prints settings prefix before user input. 
 * 
 */
void print_prefix_settings();

/**
 * @brief Prints main prefix before user input. 
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
 * @brief Connects to the server. 
 * 
 */
void cmd_server_connect();

/**
 * @brief Disconnects from the current server. 
 * 
 */
void cmd_server_disconnect();

/**
 * @brief Prints information about server. 
 * 
 */
void cmd_server_info();

/**
 * @brief Prints server addresses history. 
 * 
 */
void cmd_server_history();

/**
 * @brief Processes Thread commands.
 * 
 */
void cmd_thread();

/**
 * @brief Create thread and make it active on success.
 * 
 */
void cmd_thread_create();

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


/* -------------- Main flow --------------- */

/**
 * @brief Loop that communicates with the server. 
 * If query queue is empty, sends LIVE to keep the connection.
 *  
 */
void query_loop();

/**
 * @brief Adds server query to the global query. 
 * 
 * @param query Query to send   
 */
void server_query_add(ServerQuery *query);

/**
 * @brief Program termination handler. Simply modifies g_working value. 
 * 
 * @param Signum 
 */
void stop_program_handle(const int signum);

/**
 * @brief Initialises program.  
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

status_t main(size_t argc, char **argv);
/* ---------------------------------------- */

#endif
