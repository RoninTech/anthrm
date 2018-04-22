CXX = c++
INCL = -I../common -I../../../include/
LIBS = -L../../../lib/ -lmylcd -lpthread -lpngstatic -lzlib

OPTIONS = -funroll-loops -O3 -finline-functions -ftree-vectorizer-verbose=1 -ftree-vectorize -floop-interchange -floop-strip-mine -fomit-frame-pointer -pipe -s -DUSE_MMX -DHAVE_MMX -DHAVE_MMX2 -mmmx

CXXFLAGS = $(OPTIONS) $(INCL) -D__WIN32__=1 -D_WIN32=1 -DWIN32=1 -mconsole 

SRCS =	../common/conio.cpp	\
	../common/texture.cpp	\
	../common/timer.cpp	\
	../common/pack.cpp \
	../common/vga.cpp	\
	$(PROGNAME).cpp


OBJS =	../common/conio.o	\
	../common/texture.o	\
	../common/pack.o \
	../common/timer.o	\
	../common/vga.o		\
	$(PROGNAME).o

all:	$(PROGNAME)

$(PROGNAME):	$(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(PROGNAME)
	strip $(PROGNAME).exe
	copy "$(PROGNAME).exe" "../$(PROGNAME).exe" /y

clean:
	rm -f $(OBJS) $(PROGNAME)
