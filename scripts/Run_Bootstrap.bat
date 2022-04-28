@echo off

pushd ..\vendor\bootstrapping\
echo Running bootstrapping...
python bootstrap.py --bootstrap-file "../config/bootstrapping/bootstrap.json"
echo Copying files...
xcopy .\src ..\ /Q /S /I /R /Y
echo Done
popd