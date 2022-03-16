CFLAGS      = "-I."
LIBS_SERVER = "-pthread"
LIBS_CLIENT = "-pthread"
LIB_FILES   = $(wildcard lib/*.c)

server: server/main.c $(LIB_FILES)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -o server/LeForumServer

client: client/main.c $(LIB_FILES)
	$(CC) $(CFLAGS) $(LIBS_CLIENT) $^ -o server/LeForumClient

forum-core-test: test/forum-core.c $(LIB_FILES)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -o test/LeForumCoreTest
