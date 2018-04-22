rem @echo off
call gccpath.bat
set outexe=Ant_HRM64.exe
copy bin\x64\mylcd.dll .
rem if x86_64 add -march=k8 -mtune=k8
rem when on windows, add -mwindows for a consoleless app or remove for debug printfs
windres hrm.rc -O coff -o hrm64.res
gcc.exe -Wall -Werror -L"lib/x64/" -I"include/x64/" -mwindows -march=k8 -mtune=k8 -O2 -std=gnu99 anthrm.c console.c garminhr.c libantplus.c -lusb -lwinmm -lmylcddll -o %outexe% hrm64.res
rem gcc.exe -Wall -Werror -L"lib/x64/" -I"include/x64/" -march=k8 -mtune=k8 -O2 -std=gnu99 anthrm.c console.c garminhr.c libantplus.c -lusb -lwinmm -lmylcddll -o %outexe% hrm64.res
strip %outexe%
mkdir x64
copy %outexe% .\x64
copy mylcd.dll .\x64
copy readme.txt .\x64
copy *.ini .\x64

