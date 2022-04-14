#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <ncurses.h>

#include "lib/constants.h"
#include "lib/status.h"

struct LeLayoutPart {
	WINDOW   *win;
	double    y_size_ratio;
	double    x_size_ratio;
	int       y_size;
	int       x_size;
	int       y_coord;
	int       x_coord;
};
typedef struct LeLayoutPart LeLayoutPart;
