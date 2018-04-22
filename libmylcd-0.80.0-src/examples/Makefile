

CC=gcc
#CFLAGS = -I../include/ -Wall -mtune=native -march=native -O3 -D_WIN32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 -funroll-loops -finline-functions -ftree-vectorizer-verbose=1 -ftree-vectorize -floop-interchange -floop-strip-mine -fomit-frame-pointer -pipe -s -DUSE_MMX -DHAVE_MMX -DHAVE_MMX2 -mmmx -msse -msse2 -msse3 -m3dnow -mfpmath=387,sse -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched 
CFLAGS = -I../include/ -Wall -O3 -D_WIN32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 -funroll-loops -finline-functions -ftree-vectorizer-verbose=1 -ftree-vectorize -floop-interchange -floop-strip-mine -fomit-frame-pointer -pipe -s -DUSE_MMX -DHAVE_MMX -DHAVE_MMX2 -mmmx -msse -msse2 -msse3 -m3dnow -mfpmath=387,sse -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched 
#CFLAGS = -g -I../include/ -D_WIN32_WINNT=0x0500 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 

LIBS = -L"lib" -L../lib/ -L../libs-extra/ -lm -lmylcd -lwinmm -liphlpapi -lwininet -lwsock32 -lgdi32 -lws2_32 -lpthreadGC2 -lpsapi -lhid -lsetupapi


EXAMPLES=bandwidth.exe rssfeed.exe unicode.exe model.exe ball.exe mmx.exe \
		atitray.exe cube.exe glfwparticles.exe calendar.exe kneegolcd.exe \
		lcdtest.exe lcmd.exe particles.exe rotatetest.exe scroll.exe fire.exe \
		sysinfo.exe template.exe tgatest.exe torus.exe bdfglyph.exe quickfox.exe \
		glyphs.exe bdftest.exe filerender.exe bdfdump.exe trenderfilter.exe \
		mystify.exe supports.exe ledcard.exe plasma.exe enumdrv.exe enumlang.exe \
		trail.exe trail2.exe trail3.exe fraps.exe q2info.exe mask1.exe wolf3d.exe \
		txt2img.exe mask2.exe glyphframes.exe screencap.exe imagetest.exe usbd480BL.exe \
		enumfonts.exe ray1bpp.exe frame2h.exe edgefill.exe floodfill1.exe floodfill2.exe \
		hello.exe floatingpoint.exe gimpbitmap.exe roto.exe morph.exe flame.exe stars.exe \
		scmem.exe filter.exe alpha.exe alphatest.exe contrast.exe touch.exe bench.exe
		

all : $(EXAMPLES)


%.exe: %.o
	$(CC) -o $@ $^ $(LIBS) 
	strip.exe $@

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
		
model.o : model.c
	$(CC) $ -fexpensive-optimizations -I../include/ -O6 -mmmx -msse -funroll-loops -finline-functions -ffast-math -pipe -fomit-frame-pointer -s -std=gnu99 -c $^ -o $@

clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



