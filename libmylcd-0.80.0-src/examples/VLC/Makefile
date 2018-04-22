

CC=gcc

# Graphite loop optimizations
GLOP = -ftree-vectorize -floop-interchange -floop-strip-mine -floop-block

#CFLAGS = -g -I../../include/ -Isdk/include/ -D_DEBUG_=1 $(GLOP) -DHAVE_MMX -DHAVE_MMX2 -mmmx -std=gnu99 -Wall -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -mtune=generic -march=i686 -funroll-loops -pipe  -ftree-vectorizer-verbose=1 -fvariable-expansion-in-unroller -ftree-loop-distribution -fno-math-errno -ffast-math -msse -msse2 -msse3 -m3dnow -mfpmath=sse 
#CFLAGS = -pg -I../../include/ -Isdk/include/ -D_DEBUG_=1 $(GLOP) -march=pentium4 -mtune=pentium4 -minline-all-stringops -std=gnu99 -fno-strict-aliasing -fgnu89-inline -D__USE_MINGW_ANSI_STDIO=1 -D__MSVCRT_VERSION__=0x0601 -DHAVE_MMX -DHAVE_MMX2 -mmmx -msse -Wall -Werror -O3 -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -funroll-loops -finline-functions -ftree-vectorizer-verbose=1 -fvariable-expansion-in-unroller -ftree-loop-distribution -fno-math-errno -ffast-math -msse2 -mfpmath=387,sse -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched 
CFLAGS = -I../../include/ -Isdk/include/ $(GLOP) -march=pentium4 -mtune=pentium4 -minline-all-stringops -std=gnu99 -fno-strict-aliasing -fno-exceptions -fgnu89-inline -D__USE_MINGW_ANSI_STDIO=1 -D__MSVCRT_VERSION__=0x0601 -DHAVE_MMX -DHAVE_MMX2 -mmmx -msse -Wall -Werror -O3 -D_WIN32 -D_M_IX86 -mwin32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -funroll-loops -finline-functions -fomit-frame-pointer -ftree-vectorizer-verbose=1 -fvariable-expansion-in-unroller -ftree-loop-distribution -fno-math-errno -ffast-math -s -pipe -msse2 -mfpmath=387,sse -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched 


LIBS = -L"lib" -L../../lib/ -Lsdk/lib/

# -mtune=generic 
# -flto -mwindows

PROGRAMS=vlcstream.exe


all : $(PROGRAMS)

vlcstream.exe:	vlcstream.o browser.o vlc.o clock.o sort.o playlist.o common.o\
				cfg.o morph.o sub.o imagec.o imgovr.o chapter.o meta.o exit.o\
				winm.o tags.o playlistc.o drawvolume.o button.o zoom.o playlist2.o\
				m3u.o fileal.o ploverlay.o playlistsort.o editbox.o playlistmanager.o\
				playlistplm.o cmdparser.o lorenz.o rollc.o particles.o\
				opengl/gl_lorenz.o opengl/gl_particles.o opengl/gl_rc.o\
				opengl/matrix.o opengl/defrc.o opengl/mylcdgl.o\
				tetris.o tetris/twrap.o tetris/stc.o
	windres -i vlcstream.rc -o res.o
	$(CC) $(LIBS) -o $@ $^ res.o hook/libhook.a -lm -lglfw -lopengl32 -lglu32 -lmylcd -lvlc.dll -lvlccore.dll -lgdi32 -lwinmm -lshlwapi 
	strip.exe $@

drawvolume.o: drawvolume.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

clean:
	rm -f *.exe *.o *.bak
	rm -f vlcstream.exe


### PHONY define
.PHONY: all all-before all-after clean clean-custom



