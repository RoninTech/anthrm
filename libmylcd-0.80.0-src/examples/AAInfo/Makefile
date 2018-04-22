

CC=gcc
CFLAGS = -I../../include/ -Wall -O3 -fomit-frame-pointer -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops  -finline-functions -pipe -s
#CFLAGS = -g -I../../include/ -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 
LIBS = -L"lib" -L../../lib/
  

EXAMPLES=aainfo.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lmylcd -liphlpapi -lwininet -lwsock32 -lgdi32 -lws2_32 -lpthreadGC2 -lwinmm -lpsapi -lhid -lsetupapi 
	strip.exe $@
	copy $@ "../$@"
	copy aaserverlist.txt "../aaserverlist.txt"
	rm -f $@

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

aainfo.exe: lock.o net.o aainfo.o touch.o


clean :
	rm -f *.exe *.o *.bak



### PHONY define
.PHONY: all all-before all-after clean clean-custom



