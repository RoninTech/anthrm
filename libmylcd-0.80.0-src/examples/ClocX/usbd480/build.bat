@echo off

rem png, lglcd and sdl are not require to build this appl
rem if you experience link errors stating otherwise then rebuild libmylcd without enabling the above options

rem call gccpath.bat

gcc -I../../../include/ -c -O3 ClocXPlugin.c
dllwrap -L../../../lib/ ClocXPlugin.o -lmylcdstatic -lgdi32 -lhid -lsetupapi -o usbd480plugin.dll
strip usbd480plugin.dll

