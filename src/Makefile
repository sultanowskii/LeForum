CFLAGS = -I. -Wno-incompatible-pointer-types -Wno-incompatible-pointer-types -Wno-int-conversion

ifdef DEBUG
CFLAGS = -I. -g -Wall -Wextra
endif

LIBS_SERVER = -pthread
LIBS_CLIENT = -pthread

LIB_FILES_SERVER_SRCS = $(wildcard lib/*.c server/*.c)
LIB_FILES_SERVER_OBJS = $(patsubst %.c, %.o, $(LIB_FILES_SERVER_SRCS))
OBJS += $(LIB_FILES_SERVER_OBJS)

LIB_FILES_CLIENT_SRCS = $(wildcard lib/*.c client/*.c)
LIB_FILES_CLIENT_OBJS = $(patsubst %.c, %.o, $(LIB_FILES_CLIENT_SRCS))
OBJS += $(LIB_FILES_CLIENT_OBJS)

LIB_FILES_TEST_FORUM_CORE_SRCS = $(wildcard lib/*.c server/forum_io.c)
LIB_FILES_TEST_FORUM_CORE_OBJS = $(patsubst %.c, %.o, $(LIB_FILES_TEST_FORUM_CORE_SRCS))
OBJS += $(LIB_FILES_TEST_FORUM_CORE_OBJS)

LIB_FILES_TEST_QUEUE_SRCS = $(wildcard lib/*.c)
LIB_FILES_TEST_QUEUE_OBJS = $(patsubst %.c, %.o, $(LIB_FILES_TEST_QUEUE_SRCS))
OBJS += $(LIB_FILES_TEST_QUEUE_OBJS)

LIB_FILES_TEST_LE_STRCAT_SRCS = $(wildcard lib/*.c)
LIB_FILES_TEST_LE_STRCAT_OBJS = $(patsubst %.c, %.o, $(LIB_FILES_TEST_LE_STRCAT_SRCS))
OBJS += $(LIB_FILES_TEST_LE_STRCAT_OBJS)

all: c-server

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: c-server
c-server: server/server.o $(LIB_FILES_SERVER_OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS_SERVER) -o ../build/LeForumServer

.PHONY: c-client
c-client: client/client.o $(LIB_FILES_CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS_CLIENT) -o ../build/LeForumClient

.PHONY: c-test-forum-core
c-test-forum-core: test/test-forum-core.o $(LIB_FILES_TEST_FORUM_CORE_OBJS)
	$(CC) $(CFLAGS_G) $^ -o test/LeForumCoreTest

.PHONY: c-test-queue
c-test-queue: test/test-queue.o $(LIB_FILES_TEST_FORUM_CORE_OBJS)
	$(CC) $(CFLAGS_G) $^ -o test/QueueTest

.PHONY: c-test-le-strcat
c-test-le-strcat: test/test-le-strcat.o $(LIB_FILES_TEST_LE_STRCAT_OBJS)
	$(CC) $(CFLAGS_G) $^ -o test/strcatTest

.PHONY: clean
clean:
	rm -rf $(OBJS)

.PHONY: install
install: c-server c-client
	install -m 755 ../build/LeForumServer /usr/bin/
	install -m 755 ../build/LeForumClient /usr/bin/
