SOURCES_DIR = ./sources
INCLUDES_DIR = ./includes
BIN_DIR = ./executables

SERVER_NAME = server
SERVER_SOURCES = $(SOURCES_DIR)/protocole.c $(SOURCES_DIR)/server.c
SERVER_OPT = -lpthread

CLIENT_NAME = client
CLIENT_SOURCES = $(SOURCES_DIR)/client.c

CC = gcc
CC_FLAG = -Werror -Wextra -Wall

all: $(SERVER_NAME) $(CLIENT_NAME)

$(SERVER_NAME):	$(SERVER_SOURCES)
	$(CC) $(CC_FLAG) -I $(INCLUDES_DIR) $(SERVER_SOURCES) -o $(BIN_DIR)/$(SERVER_NAME) $(SERVER_OPT)

$(CLIENT_NAME):	$(CLIENT_SOURCES)
	$(CC) $(CC_FLAG) -I $(INCLUDES_DIR) $(CLIENT_SOURCES) -o $(BIN_DIR)/$(CLIENT_NAME)

clean:
	@rm -f $(BIN_DIR)/$(SERVER_NAME)
	@rm -f $(BIN_DIR)/$(CLIENT_NAME)

re:	clean all

.PHONY: all clean re
