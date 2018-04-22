@echo off


rem call gccpath.bat

gcc -I../../../include/ -c -O3 ClocXPlugin.c
dllwrap -L../../../lib/ ClocXPlugin.o -lmylcdstatic -lgdi32 -lhid -llglcd -lsetupapi -o g19plugin.dll
strip g19plugin.dll