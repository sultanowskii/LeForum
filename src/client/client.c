#include "client/client.h"

bool_t g_working = TRUE;

void layout_update(LeLayoutBlock *sidebar, LeLayoutBlock *main_content) {
	int            size_y, size_x;
	bool_t         sidebar_on_right;


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

	sidebar_on_right = ((SidebarData *)sidebar->current_state->data)->sidebar_on_right;

	if (sidebar_on_right) {
		sidebar->x_coord = 0;
		main_content->x_coord = sidebar->x_size;
	}
	else {
		sidebar->x_coord = main_content->x_size;
		main_content->x_coord = 0;
	}
}

void sidebar_update(LeLayoutBlock *sidebar) {
	werase(sidebar->win);

	wresize(sidebar->win, sidebar->y_size, sidebar->x_size);

	mvwin(sidebar->win, sidebar->y_coord, sidebar->x_coord);

	box(sidebar->win, 0, 0);

	wmove(sidebar->win, 0, 0);
	wrefresh(sidebar->win);
}

void main_content_example_update(LeLayoutBlock *main_content) {
	MainContentExampleData  *context;
	size_t                   text_size;


	context = (MainContentExampleData *)main_content->current_state->data;

	werase(main_content->win);

	wresize(main_content->win, main_content->y_size, main_content->x_size);

	mvwin(main_content->win, main_content->y_coord, main_content->x_coord);

	wattron(main_content->win, COLOR_PAIR(PAIR_RED_BLACK));
	box(main_content->win, 0, 0);
	wattroff(main_content->win, COLOR_PAIR(PAIR_RED_BLACK));

	if (context->text != nullptr) {
		text_size = strlen(context->text);
		wmove(main_content->win, main_content->y_size / 2, main_content->x_size / 2 - text_size / 2);
		wprintw(main_content->win, context->text);
	}
	else {
		wmove(main_content->win, main_content->y_size / 2, main_content->x_size / 2 - 6);
		wprintw(main_content->win, "Hello world!");
	}

	wmove(main_content->win, 0, 0);
	wrefresh(main_content->win);
}

void main_content_handle(LeLayoutBlock *main_content, int ch) {
	MainContentExampleData  *context;


	context = (MainContentExampleData *)main_content->current_state->data;
	switch(ch) {
		case KEY_RESIZE: {
			main_content_example_update(main_content);
			break;
		}
		case '+': {
			if (context->text != nullptr) {
				free(context->text);
				context->text = nullptr;
			}

			context->text = malloc(256);
			memset(context->text, 0, 256);

			wgetnstr(main_content->win, context->text, 255);

			main_content_example_update(main_content);
			break;
		}
		case '-': {
			if (context->text != nullptr) {
				free(context->text);
				context->text = nullptr;
			}

			main_content_example_update(main_content);
			break;
		}
	}
}

status_t sidebar_init(LeLayoutBlock **sidebar) {
	LeState                 *lestate;
	SidebarData             *data;


	*sidebar = (LeLayoutBlock *)malloc(sizeof(LeLayoutBlock));
	(*sidebar)->win = newwin(0, 0, 0, 0);
	(*sidebar)->y_size_ratio = 1;
	(*sidebar)->x_size_ratio = 0.3;
	(*sidebar)->y_size  = 0;
	(*sidebar)->x_size  = 0;
	(*sidebar)->states = queue_create(lestate_delete);

	/* Default state init */
	data = (SidebarData *)malloc(sizeof(SidebarData));
	lestate = (LeState *)malloc(sizeof(LeState));
	lestate->id = ssid_DEFAULT;
	lestate->update = sidebar_update;
	lestate->handle = NULL;
	lestate->data = data;
	{
		data->sidebar_on_right = TRUE;
	}
	lestate->data_destruct = sidebardata_delete;
	queue_push((*sidebar)->states, lestate, sizeof(LeState));

	/* It will be changed in the future. */
	(*sidebar)->current_state = (*sidebar)->states->last->data;

	return LESTATUS_OK;
}

status_t main_content_init(LeLayoutBlock **main_content) {
	LeState                 *lestate;
	MainContentExampleData  *data;


	*main_content = (LeLayoutBlock *)malloc(sizeof(LeLayoutBlock));
	(*main_content)->win = newwin(0, 0, 0, 0);
	(*main_content)->y_size_ratio = 1;
	(*main_content)->x_size_ratio = 0.7;
	(*main_content)->y_size  = 0;
	(*main_content)->x_size  = 0;
	(*main_content)->states = queue_create(lestate_delete);

	/* Init example state */
	data = (MainContentExampleData *)malloc(sizeof(MainContentExampleData));
	lestate = (LeState *)malloc(sizeof(LeState));
	lestate->id = mcsid_EXAMPLE;
	lestate->update = main_content_example_update;
	lestate->handle = main_content_handle;
	lestate->data = data;
	{
		data->text = nullptr;
	}
	lestate->data_destruct = maincontextexampledata_delete;
	queue_push((*main_content)->states, lestate, sizeof(LeState));

	/* It will be changed in the future. */
	(*main_content)->current_state = (*main_content)->states->last->data;

	return LESTATUS_OK;
}

status_t lelayoutblocks_init(LeLayoutBlock **sidebar, LeLayoutBlock **main_content) {
	sidebar_init(sidebar);
	main_content_init(main_content);
}

status_t startup(LeLayoutBlock **sidebar, LeLayoutBlock **main_content) {
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

	init_pair(PAIR_RED_BLACK, COLOR_RED, COLOR_BLACK);

	lelayoutblocks_init(sidebar, main_content);

	refresh();

	layout_update(*sidebar, *main_content);

	/* updating blocks, showing windows */
	(*main_content)->current_state->update(*main_content);
	(*sidebar)->current_state->update(*sidebar);
}

status_t cleanup(LeLayoutBlock **sidebar, LeLayoutBlock **main_content) {
	if (sidebar != nullptr && *sidebar != nullptr) {
		lelayoutblock_delete(*sidebar);
		*sidebar = nullptr;
	}

	if (main_content != nullptr && *main_content != nullptr) {
		lelayoutblock_delete(*main_content);
		*main_content = nullptr;
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
	LeLayoutBlock      *sidebar;
	LeLayoutBlock      *main_content;


	startup(&sidebar, &main_content);

	while (g_working) {
		ch = getch();

		if (ch == ERR) {
			continue;
		}

		layout_update(sidebar, main_content);

		switch (ch) {
			case ' ': {
				((SidebarData *)sidebar->current_state->data)->sidebar_on_right = !((SidebarData *)sidebar->current_state->data)->sidebar_on_right;
				layout_update(sidebar, main_content);
				main_content->current_state->update(main_content);
				sidebar->current_state->update(sidebar);
				break;
			}
			default: {
				main_content->current_state->handle(main_content, ch);
				break;
			}

		}
	}

	cleanup(&sidebar, &main_content);

	return LESTATUS_OK;
}
