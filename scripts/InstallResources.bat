@echo off

pushd ..\Mars-Sandbox\rsc\

echo Copying files... Debug Only
xcopy .\textures\example\example.png    ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\textures\example\   /Q /S /I /R /Y
xcopy .\models\bunny\bunny.obj          ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\models\bunny\       /Q /S /I /R /Y
xcopy .\shaders\vkPVPDefault.vert       ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\shaders\            /Q /S /I /R /Y
xcopy .\shaders\vkPVPCanvas.frag        ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\shaders\            /Q /S /I /R /Y
xcopy .\shaders\vkPVPCanvas.vert        ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\shaders\            /Q /S /I /R /Y
xcopy .\shaders\vkPVPDefault.frag       ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\shaders\            /Q /S /I /R /Y
xcopy .\shaders\vkPVPDefault.geom       ..\..\bin\Debug-windows-x86_64\Mars-Sandbox\rsc\shaders\            /Q /S /I /R /Y
echo Finish

popd