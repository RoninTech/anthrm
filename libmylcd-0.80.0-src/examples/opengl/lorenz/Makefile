

CC=gcc
#CFLAGS = -I../../../include/ -Wall -O3 -fomit-frame-pointer -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe 
CFLAGS = -I../../../include/ -g -Wall -O3 -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe -s
#CFLAGS = -g -I../../../include/ -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 
LIBS = -L"lib" -L../../../lib/ -L../../../libs-extra/
  

EXAMPLES=gl_lorenz.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lopengl32 -lglu32 -lmylcd -lgdi32 -lwinmm
	strip.exe $@
	@copy "$@" "../../$@"

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


gl_lorenz.exe: gl_lorenz.o ../mylcdgl.o ../file.o
clean :
	rm -f *.exe *.o *.bak
	rm -f usbd480_plugin.dll



### PHONY define
.PHONY: all all-before all-after clean clean-custom



