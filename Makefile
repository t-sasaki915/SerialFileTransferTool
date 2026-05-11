CC = gcc
STRIP = strip

# The '-Wpedantic' option may have to be removed in older compilers.
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -mwindows -Os
LDFLAGS = -static -static-libgcc -lcomctl32 -lgdi32 -lcomdlg32 -lole32

BIN_NAME = SerialFileTransferTool.exe

C_SOURCES = src/Error.c src/Main.c src/Serial.c src/UI.c src/Util.c
C_HEADERS = src/Error.h src/Serial.h src/UI.h src/Util.h src/Version.h

all: $(BIN_NAME) $(C_SOURCES) $(C_HEADERS)

$(BIN_NAME): $(C_SOURCES) $(C_HEADERS)
	$(CC) $(CFLAGS) -o $(BIN_NAME) $(C_SOURCES) $(LDFLAGS)
	$(STRIP) $(BIN_NAME)

clean:
	del $(BIN_NAME)
