@echo off

pushd ..\vendor\bootstrapping\

echo Running bootstrapping...
python bootstrap.py --bootstrap-file "../config/bootstrapping/bootstrap.json"

echo Copying files...
xcopy .\src\glfw\               ..\glfw\            /Q /S /I /R /Y
xcopy .\src\premake5\           ..\premake5\        /Q /S /I /R /Y
xcopy .\src\glm\glm\            ..\glm\glm\         /Q /S /I /R /Y
xcopy .\src\glm\util\           ..\glm\glm\util\    /Q /S /I /R /Y
xcopy .\src\stb\*.h             ..\stb\stb\         /Q /S /I /Y
xcopy .\src\stb\LICENSE         ..\stb\stb\         /Q /S /I /R /Y
xcopy .\src\imgui\              ..\imgui\imgui\     /Q /S /I /R /Y
xcopy .\src\easy_profiler\      ..\easy_profiler\   /Q /S /I /R /Y

echo Done
popd