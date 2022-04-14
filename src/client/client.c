#include "client/client.h"

LeLayoutPart  sidebar;
LeLayoutPart  content;

bool_t        sidebar_on_right;
char         *text                = nullptr;


void layout_update() {
	int            size_y, size_x;


	getmaxyx(stdscr, size_y, size_x);

	sidebar.y_size = size_y * sidebar.y_size_ratio;
	sidebar.x_size = size_x * sidebar.x_size_ratio;

	content.y_size = size_y * content.y_size_ratio;
	content.x_size = size_x * content.x_size_ratio;

	if (sidebar.x_size + content.x_size > size_x) {
		sidebar.x_size--;
	}

	if (sidebar.x_size + content.x_size < size_x) {
		content.x_size++;
	}

	sidebar.y_coord = 0;

	content.y_coord = 0;

	if (sidebar_on_right) {
		sidebar.x_coord = 0;
		content.x_coord = sidebar.x_size;
	}
	else {
		sidebar.x_coord = content.x_size;
		content.x_coord = 0;
	}
}

void sidebar_update() {
	werase(sidebar.win);

	wresize(sidebar.win, sidebar.y_size, sidebar.x_size);

	mvwin(sidebar.win, sidebar.y_coord, sidebar.x_coord);

	box(sidebar.win, 0, 0);

	wmove(sidebar.win, 0, 0);
	wrefresh(sidebar.win);
}

void content_update() {
	size_t         text_size;


	werase(content.win);

	wresize(content.win, content.y_size, content.x_size);

	mvwin(content.win, content.y_coord, content.x_coord);

	wattron(content.win,COLOR_PAIR(1));
	box(content.win, 0, 0);
	wattroff(content.win,COLOR_PAIR(1));
	
	if (text != nullptr) {
		text_size = strlen(text);
		wmove(content.win, content.y_size / 2, content.x_size / 2 - text_size / 2);
		wprintw(content.win, text);
	}
	else {
		wmove(content.win, content.y_size / 2, content.x_size / 2 - 6);
		wprintw(content.win, "Hello world!");
	}

	wmove(content.win, 0, 0);
	wrefresh(content.win);
}

void content_win_handle_input(int ch) {
	switch(ch) {
		case KEY_RESIZE: {
			content_update();
			break;
		}
		case '+': {
			if (text != nullptr) {
				free(text);
				text = nullptr;
			}

			text = malloc(256);
			memset(text, 0, 256);

			getnstr(text, 255);

			content_update();
			break;
		}
		case '-': {
			if (text != nullptr) {
				free(text);
				text = nullptr;
			}

			content_update();
			break;
		}
	}
}

status_t startup() {
	sidebar.y_size_ratio = 1;
	sidebar.x_size_ratio = 0.3;
	sidebar.y_size  = 0;
	sidebar.x_size  = 0;
	sidebar.win = newwin(0, 0, 0, 0);

	content.y_size_ratio = 1;
	content.x_size_ratio = 0.7;
	content.y_size  = 0;
	content.x_size  = 0;
	content.win = newwin(0, 0, 0, 0);

	sidebar_on_right = TRUE;

	init_pair(1, COLOR_RED, COLOR_BLACK);

	refresh();

	layout_update();

	content_update(NULL);
	sidebar_update();
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

	startup();


	while (TRUE) {
		ch = getch();
		
		layout_update();


		switch (ch) {
			case ' ': {
				sidebar_on_right = !sidebar_on_right;
				layout_update();
				content_update();
				sidebar_update();
				break;
			}
			default: {
				content_win_handle_input(ch);
				break;
			}

		}
		// sidebar_update();
	}

	delwin(content.win);
	endwin();

	return LESTATUS_OK;
}
