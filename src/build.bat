@echo off

REM call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cls

mkdir ..\build
pushd ..\build

REM cl -FC -Zi ..\src\main.cpp ..\src\RectCut.cpp ..\src\tareas.cpp ..\src\GUI.cpp /I..\include\SDL2 /I..\include ..\lib\SDL2.lib ..\lib\SDL2main.lib ..\lib\SDL2_image.lib ..\lib\SDL2_ttf.lib Shell32.lib /link /SUBSYSTEM:WINDOWS

cl -FC /O2 /Felista_tareas_sdl ..\src\main.cpp ..\src\RectCut.cpp ..\src\tareas.cpp ..\src\GUI.cpp /I..\include\SDL2 /I..\include ..\lib\SDL2.lib ..\lib\SDL2main.lib ..\lib\SDL2_image.lib ..\lib\SDL2_ttf.lib Shell32.lib /link /SUBSYSTEM:WINDOWS

popd