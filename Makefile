CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -mwindows
LDFLAGS = -static -static-libgcc -lgdi32 -lcomdlg32

BIN_NAME = SerialFileTransferTool.exe

all: $(BIN_NAME) src/*.*

$(BIN_NAME): src/*.*
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN_NAME) src/Main.c

clean:
	del $(BIN_NAME)
