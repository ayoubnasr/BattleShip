# Directories
SRC = src
INC = include
BIN = bin

CC = gcc
CFLAGS = -I$(INC) -std=gnu11 -Wall -Wno-unused-variable -Wno-unused-label -Wno-unknown-pragmas
LDLIBS = -lm

# Source files
SRCs = $(SRC)/coordslib.c $(SRC)/defs.c $(SRC)/Driver.c $(SRC)/InputLib.c $(SRC)/ShipPlacement.c $(SRC)/ShortcutFuncs.c $(SRC)/Attacks.c $(SRC)/Player.c $(SRC)/UITools.c $(SRC)/BinomialHeap.c $(SRC)/Bot.c $(SRC)/CalcProbs.c $(SRC)/D_LinkedList.c

# Platform specifics (Windows: MinGW make running in cmd)
ifeq ($(OS),Windows_NT)
    EXE = .exe
    MKDIR_BIN = if not exist $(BIN) mkdir $(BIN)
    RM_BIN = if exist $(BIN) rmdir /S /Q $(BIN)
else
    EXE =
    MKDIR_BIN = mkdir -p $(BIN)
    RM_BIN = rm -rf $(BIN)
endif

# Output executable
OUTPUT = $(BIN)/battleship$(EXE)

.PHONY: all run clean

all: $(OUTPUT)

# Compile and link
$(OUTPUT): $(SRCs) $(wildcard $(INC)/*.h)
	$(MKDIR_BIN)
	$(CC) $(CFLAGS) -o $@ $(SRCs) $(LDLIBS)

run: $(OUTPUT)
	./$(OUTPUT)

# Clean up
clean:
	$(RM_BIN)
