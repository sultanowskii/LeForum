#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <ncurses.h>

#include "lib/status.h"

struct LeContainer {
	WINDOW        *win;
	int            size_y;
	int            size_x;
	void         (*update)(struct LeContainer *);
};
typedef struct LeContainer LeContainer;