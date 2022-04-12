#include "client/client.h"

void update_win_example(LeContainer *container) {
	getmaxyx(stdscr, container->size_y, container->size_x);

	werase(container->win);

	wresize(container->win, container->size_y, container->size_x);
	box(container->win, 0, 0);
	wmove(container->win, container->size_y / 2, container->size_x / 2 - 6);
	wprintw(container->win, "Hello world!");

	wmove(container->win, 0, 0);
	wrefresh(container->win);
} 

status_t main(size_t argc, char **argv) {
	int          ch;
	LeContainer *container_example;


	if (!initscr()) {
		perror("initscr() failed:");
		return LESTATUS_CLIB;
	}

	cbreak();
	noecho();
	curs_set(0);

	container_example = (LeContainer *)malloc(sizeof(LeContainer));
	getmaxyx(stdscr, container_example->size_y, container_example->size_x);
	container_example->win = newwin(container_example->size_y, container_example->size_x, 0, 0);
	container_example->update = update_win_example;

	refresh();
	container_example->update(container_example);

	while (TRUE) {
		ch = getch();
		refresh();

		if (ch == KEY_RESIZE) {
			container_example->update(container_example);
		}
	}

	delwin(container_example->win);
	endwin();

	free(container_example);

	return LESTATUS_OK;
}
