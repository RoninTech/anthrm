

CC=gcc
CFLAGS = -I../../../include/ -Wall -O3 -D_M_IX86 -mwin32 -D_STDCALL_SUPPORTED -DOPENGLUT_STATIC -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe -s
# CFLAGS = -g -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=c99 
LIBS = -L"lib" -L../../../lib/ -L../../../libs-extra/
  

EXAMPLES=distort.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lopenglut -lopengl32 -lglu32 -lmylcd -lgdi32 -lwinmm
	strip.exe $@
	@copy "$@" "../../$@"
	@copy "distort.rgb" "../../distort.rgb"
	

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

distort.exe: distort.o ripple.o ripple_table.o rubber.o texture.o ../mylcdgl.o ../file.o

clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



