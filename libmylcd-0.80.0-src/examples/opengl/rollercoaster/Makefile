

CC=gcc
CFLAGS = -Wall -O3 -I../../../include/ -D_M_IX86 -mwin32 -D_STDCALL_SUPPORTED -DOPENGLUT_STATIC -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe -s
LIBS = -L"lib" -L../../../lib/ -L../../../libs-extra/
  

EXAMPLES=rc.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lopenglut -lopengl32 -lglu32 -lmylcd -llglcd -lgdi32 -lwinmm
	strip.exe $@
	@copy "$@" "../../$@"
	@copy "rc.def" "../../rc.def"

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rc.exe: matrix.o defrc.o rc.o ../mylcdgl.o ../file.o

clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



