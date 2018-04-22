

CC=gcc
CFLAGS = -I../../include/ -Wall -O3 -D_WIN32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 -finline-functions -pipe -s
#CFLAGS = -I../../include/ -g -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 
LIBS = -L"lib" -L../../lib/


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
clock.exe: ../rss/rss.o utf8.o clock.o
	$(CC) $(LIBS) -o $@ $^ -lm -lmylcd libnetclient.a -lwininet
	strip.exe $@
	copy $@ "../$@"
	copy netclient.dll "../netclient.dll"
	
clean :
	rm -f *.exe *.o *.bak
	rm -f ../rss/rss.o



### PHONY define
.PHONY: all all-before all-after clean clean-custom



