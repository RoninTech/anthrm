@echo off

call gccpath.bat

%DEVCRM% -f hook.o hook.dll libhook.a

rem -e __DllMainCRTStartup@12

echo compiling....
%DEVCGCC% -O2 -D_WIN32_WINNT=0x0500 -march=i686 -mtune=generic -funroll-loops -c hook.c -o hook.o -I%DEVCI% -DBUILDING_DLL=1 %DEVOP%

echo linking...
%DEVCDLLW% --implib libhook.a hook.o -L%DEVCL% --no-export-all-symbols --add-stdcall-alias -o hook.dll -s

rem echo cleaning....
del hook.o
