@echo off

echo Building Dependencies...

pushd ..\vendor\config\assimp
call .\build_assimp.bat
popd

echo Done.