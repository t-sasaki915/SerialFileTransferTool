CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -mwindows
LDFLAGS = -static -static-libgcc -lgdi32 -lcomdlg32

BIN_NAME = SerialFileTransferTool.exe

C_SOURCES = src/Main.c src/Serial.c src/UI.c src/Util.c
C_HEADERS = src/Serial.h src/UI.h src/Util.h

all: $(BIN_NAME) $(C_SOURCES) $(C_HEADERS)

$(BIN_NAME): $(C_SOURCES) $(C_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN_NAME) $(C_SOURCES)

clean:
	del $(BIN_NAME)
