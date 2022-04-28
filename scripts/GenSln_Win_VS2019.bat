@echo off

pushd ..\config\
call ..\vendor\premake5\premake5.exe vs2019
popd