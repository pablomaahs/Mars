@echo off

pushd ..\config\
call ..\vendor\premake5\premake5.exe --gfxapi="vulkan" vs2019
popd