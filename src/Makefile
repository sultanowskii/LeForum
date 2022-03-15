CFLAGS = "-I."

LIBS_SERVER = "-pthread"
LIBS_CLIENT = "-pthread"

server: server/main.c
	$(CC) $(CFLAGS) $(LIBS_SERVER) server/main.c -o server/LeForumServer

client: client/main.c
	$(CC) $(CFLAGS) $(LIBS_SERVER) client/main.c -o server/LeForumClient
