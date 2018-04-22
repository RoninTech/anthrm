@echo off

rem call gccpath.bat
del *.o

gcc -I../../include/ -Wall -c usbd480.c -O3
dllwrap -L../../lib/ usbd480.o -o usbd480.dll --input-def plugin.def -lgdi32 -lmylcdstatic -lwinmm -lsetupapi -lhid -k --add-stdcall-alias
strip usbd480.dll

copy "usbd480.dll" "o:\lcdmisc\dll\usbd480.dll" /y