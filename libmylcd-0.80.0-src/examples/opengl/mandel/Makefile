

CC=g++

CPPFLAGS = -I../../../include/ -Wall -O3 -fomit-frame-pointer -D_WIN32 -D_STDCALL_SUPPORTED -DOPENGLUT_STATIC -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe -s
LIBS = -L"lib" -L../../../lib/ -L../../../libs-extra/
  

EXAMPLES=gl_mandel.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lopenglut -lopengl32 -lglu32 -lmylcd -lgdi32 -lpthreadGC2 -lwinmm
	strip.exe $@
	@copy "colourpresets" "../../colourpresets"
	@copy "$@" "../../$@"

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)


gl_mandel.exe: gl_mandel.o ../mylcdgl.o ../file.o

clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



