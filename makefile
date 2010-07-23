#
# MyGDB Makefile
# inhak.min@gmail.com
#

CC = g++
TARGET = mygdb
UNAME= $(shell uname)

ifeq ($(UNAME), MINGW32_NT-5.1)
WXCONFIG = lib/release/bin/wx-config
else
WXCONFIG = wx-config
endif

CFLAGS = -W -Wno-write-strings -I.

ifeq (ON,$(DEBUG))
	CFLAGS += -g
else
	CFLAGS += -O2
endif
    
CFLAGS += `$(WXCONFIG) --cflags`
ifeq ($(UNAME), MINGW32_NT-5.1)
	LDFLAGS =  -lwx_mswu_stc-2.8 `$(WXCONFIG) --libs` 
else
	LDFLAGS =  -lwx_gtk2u_stc-2.8 `$(WXCONFIG) --libs` 
endif

ifeq ($(UNAME), MINGW32_NT-5.1)
    LDFLAGS += -static
endif

.SUFFIXES:
.SUFFIXES: .cpp .o

SRC = 	treelistctrl.cpp \
		Console.cpp Debugger.cpp MyGDB.cpp \
		Config.cpp MyAuiDockArt.cpp notebookstyles.cpp \
		Completion.cpp STCStyle.cpp InvokeApp.cpp \
		StyledTextCtrl.cpp CodeEditor.cpp Watch.cpp \
		Register.cpp CallStack.cpp Breakpoint.cpp \
		GridCtrl.cpp Memory.cpp About.cpp

OBJECTS = $(SRC:.cpp=.o)

ifeq ($(UNAME), MINGW32_NT-5.1)
	OBJECTS += mygdb.res
endif

.cpp.o : ; $(CC) -c $(CFLAGS) -o $*.o $<

%.res : %.rc
	windres `$(WXCONFIG) --cppflags -rescomp` -i $< -J rc -o $@ -O coff

all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(UNAME), MINGW32_NT-5.1)
	$(CC) -o $@ -mthreads -Wl,--subsystem,windows -mwindows \
		$(OBJECTS) $(LDFLAGS) 
else
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) 
endif

clean:
ifeq ($(UNAME), MINGW32_NT-5.1)
	rm -f *.o *.res $(TARGET).exe
else
	rm -f *.o $(TARGET)
endif
