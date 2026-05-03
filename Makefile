CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -mwindows
BIN_NAME = SerialFileTransferTool.exe

all: $(BIN_NAME) Main.c

$(BIN_NAME): Main.c
	$(CC) $(CFLAGS) -o $(BIN_NAME) Main.c

clean:
	del $(BIN_NAME)
