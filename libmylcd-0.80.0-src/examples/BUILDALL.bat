@echo off

call gccpath.bat
make all

del opengl\mylcdgl.o
del opengl\file.o
cd opengl\rollercoaster
make
cd ..\..

cd opengl\distort
make
cd ..\..

cd opengl\glfw
make
cd ..\..

cd opengl\lorenz
make
cd ..\..

del opengl\mylcdgl.o
del opengl\file.o
cd opengl\mandel
make
cd ..\..

cd clock
make
cd ..

cd irc
make
cd..

cd aainfo
make
cd ..