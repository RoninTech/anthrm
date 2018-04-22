@echo off 

call gccpath.bat

del mylcd.def /y
ren libmylcd.def mylcd.def
lib /machine:IX86 /def:mylcd.def

