#include "client/client.h"

WINDOW *win_example;
WINDOW *win_sidebar;

double sidebar_y_ratio = 1;
double sidebar_x_ratio = 0.3;

double content_y_ratio = 1;
double content_x_ratio = 0.7;


void win_example_update(char *text) {
	static int     size_y, size_x;
	static int     max_size_y, max_size_x;
	size_t         n;


	getmaxyx(stdscr, max_size_y, max_size_x);

	werase(win_example);

	size_y = max_size_y;
	size_x = max_size_x * content_x_ratio;

	wresize(win_example, size_y, size_x);

	mvwin(win_example, 0, max_size_x - size_x);

	wattron(win_example,COLOR_PAIR(1));
	box(win_example, 0, 0);
	wattroff(win_example,COLOR_PAIR(1));
	
	if (text != nullptr) {
		n = strlen(text);
		wmove(win_example, size_y / 2, size_x / 2 - n / 2);
		wprintw(win_example, text);
	}
	else {
		wmove(win_example, size_y / 2, size_x / 2 - 6);
		wprintw(win_example, "Hello world!");
	}

	wmove(win_example, 0, 0);
	wrefresh(win_example);
}

void win_example_handle_input(int ch) {
	static char        *tmp_text = nullptr;


	switch(ch) {
		case KEY_RESIZE: {
			win_example_update(tmp_text);
			break;
		}
		case '+': {
			if (tmp_text != nullptr) {
				free(tmp_text);
				tmp_text = nullptr;
			}

			tmp_text = malloc(256);
			memset(tmp_text, 0, 256);

			getnstr(tmp_text, 255);

			win_example_update(tmp_text);
			break;
		}
		case '-': {
			if (tmp_text != nullptr) {
				free(tmp_text);
				tmp_text = nullptr;
			}

			win_example_update(tmp_text);
			break;
		}
	}
}


status_t main(size_t argc, char **argv) {
	int          ch;
	int          tmp_y, tmp_x;
	

	if (!initscr()) {
		perror("initscr() failed:");
		return LESTATUS_CLIB;
	}

	/* anti-raw() */
	cbreak();
	/* input is not printed */
	noecho();
	/* hides cursor */
	curs_set(0);

	/* starts using colors, helpful for --no-color implementation :) */
	start_color();

	refresh();

	getmaxyx(stdscr, tmp_y, tmp_x);
	win_example = newwin(tmp_y, tmp_x, 0, 0);

	init_pair(1, COLOR_RED, COLOR_BLACK);
	win_example_update(NULL);

	while (TRUE) {
		ch = getch();

		win_example_handle_input(ch);
	}

	delwin(win_example);
	endwin();

	return LESTATUS_OK;
}
