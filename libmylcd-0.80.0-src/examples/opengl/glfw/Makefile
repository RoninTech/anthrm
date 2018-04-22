

CC=gcc
CFLAGS = -I../../../include/ -Wall -O3 -fomit-frame-pointer -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 -mtune=generic -march=i686 -funroll-loops -finline-functions -pipe
# CFLAGS = -g -D_WIN32_WINNT=0x0500 -DWINVER=0x0500 -D__WIN32__=1 -std=gnu99 
LIBS = -L"lib" -L../../../lib/ -L../../../libs-extra/
  

EXAMPLES=gl_boing.exe gl_gears.exe gl_triangle.exe gl_particles.exe

all : $(EXAMPLES)


%.exe: %.o
	$(CC) $(LIBS) -o $@ $^ -lm -lglfw -lopengl32 -lglu32 -lmylcd -lgdi32 -lwinmm
	strip.exe $@
	@copy "$@" "../../$@"

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	

gl_boing.exe: ../mylcdgl.o ../file.o gl_boing.o
gl_gears.exe: ../mylcdgl.o ../file.o gl_gears.o
gl_triangle.exe: ../mylcdgl.o ../file.o gl_triangle.o
gl_particles.exe: ../mylcdgl.o ../file.o gl_particles.o

clean :
	rm -f *.exe *.o *.bak



### PHONY define
.PHONY: all all-before all-after clean clean-custom



