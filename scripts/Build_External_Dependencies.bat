@echo off

echo Building Dependencies...

pushd ..\vendor\config\assimp
call .\build_assimp.bat
popd

pushd ..\vendor\config\vulkan_headers
call .\build_vulkan_headers.bat
popd

echo Done.