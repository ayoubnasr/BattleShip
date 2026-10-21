# Directories
SRC = src
INC = include
BIN = bin

CC = gcc
CFLAGS = -I$(INC) -std=gnu11 -Wall -Wno-unused-variable -Wno-unused-label -Wno-unknown-pragmas
LDLIBS = -lm

# Source files
SRCs = $(SRC)/coordslib.c $(SRC)/defs.c $(SRC)/Driver.c $(SRC)/InputLib.c $(SRC)/ShipPlacement.c $(SRC)/ShortcutFuncs.c $(SRC)/Attacks.c $(SRC)/Player.c $(SRC)/UITools.c $(SRC)/BinomialHeap.c $(SRC)/Bot.c $(SRC)/CalcProbs.c $(SRC)/D_LinkedList.c $(SRC)/BotView.c

# Executables get the .exe extension on Windows. (bin/ is kept in git with a .gitkeep file, so no mkdir is needed on any shell.)
ifeq ($(OS),Windows_NT)
    EXE = .exe
else
    EXE =
endif

# Output executable
OUTPUT = $(BIN)/battleship$(EXE)

# Unit tests: every source file except the driver (it holds main()), plus the files in tests/
TEST_DIR = tests
TEST_SRCs = $(filter-out $(SRC)/Driver.c, $(SRCs)) $(wildcard $(TEST_DIR)/*.c)
TEST_OUTPUT = $(BIN)/tests$(EXE)

.PHONY: all run test clean

all: $(OUTPUT)

# Compile and link
$(OUTPUT): $(SRCs) $(wildcard $(INC)/*.h)
	$(CC) $(CFLAGS) -o $@ $(SRCs) $(LDLIBS)

run: $(OUTPUT)
	./$(OUTPUT)

$(TEST_OUTPUT): $(TEST_SRCs) $(wildcard $(INC)/*.h) $(wildcard $(TEST_DIR)/*.h)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRCs) $(LDLIBS)

test: $(TEST_OUTPUT)
	./$(TEST_OUTPUT)

# Clean up (MinGW make may run recipes in cmd or in sh: on Windows try both, ignoring the one that doesn't exist)
clean:
	-rm -f $(OUTPUT) $(TEST_OUTPUT)
ifeq ($(OS),Windows_NT)
	-del /Q $(subst /,\,$(OUTPUT) $(TEST_OUTPUT)) 2>NUL
endif
