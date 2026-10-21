# Directories
SRC = src
INC = include

# Source files
SRCs = $(SRC)/coordslib.c $(SRC)/defs.c $(SRC)/Driver.c $(SRC)/InputLib.c $(SRC)/ShipPlacement.c $(SRC)/ShortcutFuncs.c $(SRC)/Attacks.c $(SRC)/Player.c $(SRC)/UITools.c $(SRC)/BinomialHeap.c $(SRC)/Bot.c $(SRC)/CalcProbs.c $(SRC)/D_LinkedList.c

# Output executable
OUTPUT = bin/main

# Compile and link
$(OUTPUT): $(SRCs)
	gcc -I$(INC) -o $@ $^

# Clean up
clean:
	del /Q $(OUTPUT)