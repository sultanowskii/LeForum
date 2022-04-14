#pragma ocnce

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
