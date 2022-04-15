#include "client/client.h"

#define ctrl(ch) ((ch) & 0x1f)

LeLayoutBlock     *sidebar;
LeLayoutBlock     *main_content;

bool_t             g_working             = TRUE;
bool_t             g_sidebar_on_right    = TRUE;


void layout_update() {
	int            size_y, size_x;


	getmaxyx(stdscr, size_y, size_x);

	sidebar->y_size = size_y * sidebar->y_size_ratio;
	sidebar->x_size = size_x * sidebar->x_size_ratio;

	main_content->y_size = size_y * main_content->y_size_ratio;
	main_content->x_size = size_x * main_content->x_size_ratio;

	if (sidebar->x_size + main_content->x_size > size_x) {
		sidebar->x_size--;
	}

	if (sidebar->x_size + main_content->x_size < size_x) {
		main_content->x_size++;
	}

	sidebar->y_coord = 0;

	main_content->y_coord = 0;

	if (g_sidebar_on_right) {
		sidebar->x_coord = 0;
		main_content->x_coord = sidebar->x_size;
	}
	else {
		sidebar->x_coord = main_content->x_size;
		main_content->x_coord = 0;
	}
}

void sidebar_update() {
	int            x, y;


	werase(sidebar->win);

	wresize(sidebar->win, sidebar->y_size, sidebar->x_size);

	mvwin(sidebar->win, sidebar->y_coord, sidebar->x_coord);

	box(sidebar->win, 0, 0);

	/* Filling sidebar with tabs (main content states) */
	x = 2;
	y = 1;

	wmove(sidebar->win, y, x);

	for (enum MainContentStateIDs id = _mcsid_BEGIN + 1; id < _mcsid_END; id++) {
		if (id == main_content->current_state->id) {
			wattron(sidebar->win, A_REVERSE);
			wprintw(sidebar->win, MainContentStateIDs_REPR(id));
			wattroff(sidebar->win, A_REVERSE);
		}
		else {
			wprintw(sidebar->win, MainContentStateIDs_REPR(id));
		}
		y++;
		wmove(sidebar->win, y, x);
	}

	wmove(sidebar->win, 0, 0);
	wrefresh(sidebar->win);
}

void sidebar_handle(int ch) {
	SidebarData   *data;


	data = (SidebarData *)sidebar->current_state->data;


	switch (ch) {
		case KEY_RESIZE: {
			sidebar->current_state->update();
			break;
		}
		case KEY_UP: {
			// s_dec(tmp, _mcsid_BEGIN + 1, _mcsid_END - 1);
			sidebar->current_state->update();
			break;
		}
		case KEY_DOWN: {
			// s_inc(tmp, _mcsid_BEGIN + 1, _mcsid_END - 1);
			sidebar->current_state->update();
			break;
		}
	}
}

void main_content_example_update() {
	MainContentExampleData  *data;
	size_t                   text_size;


	data = (MainContentExampleData *)main_content->current_state->data;

	werase(main_content->win);

	wresize(main_content->win, main_content->y_size, main_content->x_size);

	mvwin(main_content->win, main_content->y_coord, main_content->x_coord);

	wattron(main_content->win, COLOR_PAIR(PAIR_RED_BLACK));
	box(main_content->win, 0, 0);
	wattroff(main_content->win, COLOR_PAIR(PAIR_RED_BLACK));

	if (data->text != nullptr) {
		text_size = strlen(data->text);
		wmove(main_content->win, main_content->y_size / 2, main_content->x_size / 2 - text_size / 2);
		wprintw(main_content->win, data->text);
	}
	else {
		wmove(main_content->win, main_content->y_size / 2, main_content->x_size / 2 - 6);
		wprintw(main_content->win, "Hello world!");
	}

	wmove(main_content->win, 0, 0);
	wrefresh(main_content->win);
}

void main_content_example_handle(int ch) {
	MainContentExampleData  *data;


	data = (MainContentExampleData *)main_content->current_state->data;

	switch (ch) {
		case KEY_RESIZE: {
			main_content_example_update();
			break;
		}
		case '+': {
			if (data->text != nullptr) {
				free(data->text);
				data->text = nullptr;
			}

			data->text = malloc(256);
			memset(data->text, 0, 256);

			wgetnstr(main_content->win, data->text, 255);

			main_content_example_update();
			break;
		}
		case '-': {
			if (data->text != nullptr) {
				free(data->text);
				data->text = nullptr;
			}

			main_content_example_update();
			break;
		}
	}
}

status_t sidebar_init() {
	LeState                 *lestate;
	SidebarData             *data;


	sidebar = (LeLayoutBlock *)malloc(sizeof(LeLayoutBlock));
	sidebar->win = newwin(0, 0, 0, 0);
	sidebar->y_size_ratio = 1;
	sidebar->x_size_ratio = 0.3;
	sidebar->y_size  = 0;
	sidebar->x_size  = 0;
	sidebar->states = queue_create(lestate_delete);

	/* Default state init */
	data = (SidebarData *)malloc(sizeof(SidebarData));
	lestate = (LeState *)malloc(sizeof(LeState));
	lestate->id = ssid_DEFAULT;
	lestate->update = sidebar_update;
	lestate->handle = sidebar_handle;
	lestate->data = data;
	lestate->name = SidebarStateIDs_REPR(lestate->id);
	lestate->data_destruct = sidebardata_delete;
	queue_push(sidebar->states, lestate, sizeof(LeState));

	/* It will be changed in the future. */
	sidebar->current_state = sidebar->states->last->data;

	return LESTATUS_OK;
}

status_t main_content_init() {
	LeState                 *lestate;
	MainContentExampleData  *data;


	main_content = (LeLayoutBlock *)malloc(sizeof(LeLayoutBlock));
	main_content->win = newwin(0, 0, 0, 0);
	main_content->y_size_ratio = 1;
	main_content->x_size_ratio = 0.7;
	main_content->y_size  = 0;
	main_content->x_size  = 0;
	main_content->states = queue_create(lestate_delete);

	/* Init example state */
	data = (MainContentExampleData *)malloc(sizeof(MainContentExampleData));
	lestate = (LeState *)malloc(sizeof(LeState));
	lestate->id = mcsid_EXAMPLE;
	lestate->update = main_content_example_update;
	lestate->handle = main_content_example_handle;
	lestate->data = data;
	{
		data->text = nullptr;
	}
	lestate->name = MainContentStateIDs_REPR(lestate->id);
	lestate->data_destruct = maincontentexampledata_delete;
	queue_push(main_content->states, lestate, sizeof(LeState));

	/* It will be changed in the future. */
	main_content->current_state = main_content->states->last->data;

	return LESTATUS_OK;
}

status_t lelayoutblocks_init() {
	sidebar_init();
	main_content_init();
}

status_t startup() {
	signal(SIGTERM, stop_program_handle);
	signal(SIGINT, stop_program_handle);

	/* Prevents process termination on SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

	if (!initscr()) {
		perror("initscr() failed:");
		return LESTATUS_CLIB;
	}

	/* Anti-raw() */
	cbreak();
	/* Input is not printed */
	noecho();
	/* Hides cursor */
	curs_set(0);
	/* Starts using colors, helpful for --no-color implementation :) */
	start_color();
	/* Prevents getch() blocking */
	nodelay(stdscr, TRUE);
	/* For CTRL+KEY */
    keypad(stdscr, TRUE);

	g_sidebar_on_right = TRUE;
	g_working = TRUE;

	init_pair(PAIR_RED_BLACK, COLOR_RED, COLOR_BLACK);

	lelayoutblocks_init();

	refresh();

	layout_update();

	/* updating blocks, showing windows */
	main_content->current_state->update();
	sidebar->current_state->update();
}

status_t cleanup() {
	if (sidebar != nullptr) {
		lelayoutblock_delete(sidebar);
		sidebar = nullptr;
	}

	if (main_content != nullptr) {
		lelayoutblock_delete(main_content);
		main_content = nullptr;
	}

	endwin();

	return LESTATUS_OK;
}

void stop_program_handle(const int signum) {
	g_working = FALSE;
}

status_t main(size_t argc, char **argv) {
	int                 ch;
	int                 tmp_y, tmp_x;


	startup();

	main_content = main_content;
	sidebar = sidebar;

	while (g_working) {
		ch = getch();

		if (ch == ERR) {
			continue;
		}

		layout_update();

		switch (ch) {
			case ' ': {
				g_sidebar_on_right = !g_sidebar_on_right;
				layout_update();
				main_content->current_state->update();
				sidebar->current_state->update();
				break;
			}
			default: {
				sidebar->current_state->handle(ch);
				main_content->current_state->handle(ch);
				break;
			}

		}
	}

	cleanup();

	return LESTATUS_OK;
}
