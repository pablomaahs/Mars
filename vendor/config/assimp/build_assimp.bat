@echo off

set BINARIES_DIR="./build"
set GENERATOR="Visual Studio 16 2019"
set PLATFORM="x64"

set PROYECT_NAME="Mars-Sandbox"
set DEBUG_BUILD_NAME="Debug-windows-x86_64"
set RELEASE_BUILD_NAME="Release-windows-x86_64"

pushd ..\..\assimp\
echo Building Assimp...
cmake . -G %GENERATOR% -A %PLATFORM% -B %BINARIES_DIR% -D BUILD_SHARED_LIBS=ON -D ASSIMP_NO_EXPORT=ON -D ASSIMP_BUILD_DRACO=OFF -D ASSIMP_BUILD_ASSIMP_TOOLS=OFF -D ASSIMP_BUILD_TESTS=OFF -D ASSIMP_INSTALL_PDB=OFF -D ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -D ASSIMP_BUILD_OBJ_IMPORTER=ON -D ASSIMP_BUILD_GLTF_IMPORTER=ON
cmake --build %BINARIES_DIR% --config debug
cmake --build %BINARIES_DIR% --config release

echo Copying Assimp Library...
xcopy %BINARIES_DIR%\lib\Debug\*.lib ..\..\bin\%DEBUG_BUILD_NAME%\%PROYECT_NAME%\ /Q /S /I /R /Y
xcopy %BINARIES_DIR%\lib\Release\*.lib ..\..\bin\%RELEASE_BUILD_NAME%\%PROYECT_NAME%\ /Q /S /I /R /Y
xcopy %BINARIES_DIR%\bin\Debug\*.dll ..\..\bin\%DEBUG_BUILD_NAME%\%PROYECT_NAME%\ /Q /S /I /R /Y
xcopy %BINARIES_DIR%\bin\Release\*.dll ..\..\bin\%RELEASE_BUILD_NAME%\%PROYECT_NAME%\ /Q /S /I /R /Y

echo Assimp Done.
popd