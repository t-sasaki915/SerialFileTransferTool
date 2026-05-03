CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -mwindows
LDFLAGS = -static -static-libgcc -lgdi32 -lcomdlg32
BIN_NAME = SerialFileTransferTool.exe

all: $(BIN_NAME) Main.c

$(BIN_NAME): Main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN_NAME) Main.c

clean:
	del $(BIN_NAME)
