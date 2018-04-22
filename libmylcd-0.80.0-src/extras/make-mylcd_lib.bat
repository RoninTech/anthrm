@echo off 

cd %1

ren mylcd mylcd.def
lib /machine:IX86 /def:mylcd.def

