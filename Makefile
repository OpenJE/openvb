CFG = Debug
OPENVB = .
OPENJE = ..\openje

CC = cl
LINK = link

EXE = F3.exe

!IF "$(CFG)" == "Debug"
EXE = $(OPENVB)\build\F3-dbg.exe
!ELSE
EXE = $(OPENVB)\build\F3.exe
!ENDIF

INCS = /I$(OPENVB)\src \
	   /I$(OPENJE)\src

SRCS = $(OPENVB)\src\main.cpp \
	   $(OPENVB)\src\F3.cpp \
	   $(OPENJE)\src\JE.cpp

OBJS = $(OPENVB)\obj\main.obj \
	   $(OPENVB)\obj\F3.obj \
	   $(OPENJE)\obj\JE.obj

LIBS = user32.lib \
	   gdi32.lib \
	   winmm.lib

DEBUG_ARGS = /c \
			 /EHsc \
			 /Zi \
			 /Od \
			 /GS \
			 /DLOGGER \
			 /DLOG_TERMINAL \
			 /DLOG_FILE

ARGS = /c \
	   /EHsc \
	   /O2

!IF "$(CFG)" == "Debug"
CFLAGS = $(INCS) $(DEBUG_ARGS)
LDFLAGS = /DEBUG
!ELSE
CFLAGS = $(INCS) $(ARGS)
LDFLAGS =
!ENDIF

all: $(EXE)

$(OPENVB)\obj\main.obj: $(OPENVB)\src\main.cpp
	$(CC) $(CFLAGS) /Fo$(OPENVB)\obj\main.obj $(OPENVB)\src\main.cpp

$(OPENVB)\obj\F3.obj: $(OPENVB)\src\F3.cpp
	$(CC) $(CFLAGS) /Fo$(OPENVB)\obj\F3.obj $(OPENVB)\src\F3.cpp

$(OPENJE)\obj\JE.obj: $(OPENJE)\src\JE.cpp
	$(CC) $(CFLAGS) /Fo$(OPENJE)\obj\JE.obj $(OPENJE)\src\JE.cpp

$(EXE): $(OBJS)
	$(LINK) $(OBJS) $(LIBS) /OUT:$(EXE) $(LDFLAGS)

clean:
	-del $(OBJS) $(EXE)
