CC = cl
LINK = link

EXE = F3.exe

SRC = src\main.cpp \
      src\f3.cpp

OBJ = src\main.obj \
      src\f3.obj

LIB = user32.lib \
	   gdi32.lib

CFLAGS = /c /EHsc

all: $(EXE)

src\main.obj: src\main.cpp
	$(CC) $(CFLAGS) src\main.cpp

src\f3.obj: src\f3.cpp
	$(CC) $(CFLAGS) src\f3.cpp

$(EXE): $(OBJ)
	$(LINK) $(OBJ) $(LIB) /OUT:$(EXE)

clean:
	-del $(OBJ) $(EXE)
