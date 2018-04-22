

CC=gcc
CFLAGS = -I../../include/ -Wall -O3 -fomit-frame-pointer -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe
# CFLAGS = -I../../include/ -g -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 
LIBS = -L"lib" -L../../lib/
  

EXAMPLES=irc.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lmylcd -liphlpapi -lwininet -lwsock32 -lgdi32 -lws2_32 -lpthreadGC2 -lwinmm -lpsapi -lhid -lsetupapi 
	strip.exe $@
	copy $@ "../$@"
	copy irc.cfg "../irc.cfg"
	rm -f $@

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


irc.exe: lock.o network.o net.o send.o bot.o g15.o config.o irc.o


clean :
	rm -f *.exe *.o *.bak
	rm -f usbd480_plugin.dll



### PHONY define
.PHONY: all all-before all-after clean clean-custom



