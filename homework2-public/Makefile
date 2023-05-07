CC = g++
CFLAGS = -w -Wall


SERVER_SOURCE_FILES = \
	server.cpp \
	helper.cpp \

CLIENT_SOURCE_FILES = \
	tcp_client.cpp \
	helper.cpp \
 
# SINGLE_HEADERS =

all: server subscriber

SERVER_OBJ_FILES = $(SERVER_SOURCE_FILES:.c=.o)
CLIENT_OBJ_FILES = $(CLIENT_SOURCE_FILES:.c=.o)

# Compileaza server.c
server: $(SERVER_OBJ_FILES) server.cpp
	$(CC) $(CFLAGS) $^ -o $@

# Compileaza client.c
subscriber: $(CLIENT_OBJ_FILES) tcp_client.cpp
	$(CC) $(CFLAGS) $^ -o $@


.PHONY: clean run_server run_subscriber

%.o : %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f server subscriber *.o
