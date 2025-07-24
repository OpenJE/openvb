# Makefile for test.cpp Win32 GUI app

# Compiler and linker
CC = cl
LINK = link

# Source and object files
SRC = src/f3.cpp
OBJ = F3.obj
EXE = F3.exe

# Libraries
LIBS = user32.lib gdi32.lib

# Compiler flags
CFLAGS = /c /EHsc

# Linker flags
LFLAGS =

all: $(EXE)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) $(SRC)

$(EXE): $(OBJ)
	$(LINK) $(OBJ) $(LIBS) $(LFLAGS) /OUT:$(EXE)

clean:
	del $(OBJ) $(EXE)
