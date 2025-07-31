CFG = Debug

CC = cl
LINK = link

EXE = F3.exe

SRCS = src\main.cpp

OBJS = obj\main.obj

LIBS = user32.lib \
	   gdi32.lib

!IF "$(CFG)" == "Debug"
CFLAGS = /c /EHsc /Zi /Od
LDFLAGS = /DEBUG
!ELSE
CFLAGS = /c /EHsc /O2
LDFLAGS =
!ENDIF

all: $(EXE)

obj\main.obj: src\main.cpp
	$(CC) $(CFLAGS) /Foobj\main.obj src\main.cpp

$(EXE): $(OBJS)
	$(LINK) $(OBJS) $(LIBS) /OUT:$(EXE) $(LDFLAGS)

clean:
	-del $(OBJ) $(EXE)
