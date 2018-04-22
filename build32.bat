rem @echo off

call gccpath.bat
set outexe=Ant_HRM32.exe
copy bin\x32\mylcd.dll .

rem if x86_64 add -march=k8 -mtune=k8
rem when on windows, add -mwindows for a consoleless app. remove to enable console mode
windres hrm.rc -O coff -o hrm32.res
gcc -Wall -Werror -L"lib/x32/" -I"include/x32/" -mwindows -O2 -std=gnu99 anthrm.c console.c garminhr.c libantplus.c -lusb -lwinmm -lmylcddll -o %outexe% hrm32.res
rem gcc -Wall -Werror -L"lib/x32/" -I"include/x32/" -O2 -std=gnu99 anthrm.c console.c garminhr.c libantplus.c -lusb -lwinmm -lmylcddll -o %outexe%
strip %outexe%
mkdir x32
copy %outexe% .\x32
copy mylcd.dll .\x32
copy readme.txt .\x32
copy *.ini .\x32
