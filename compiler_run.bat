@ECHO off
SETLOCAL

SET DIR=%~dp0
SET BUILD_DIR=%DIR%build\
SET GEN_DIR=%DIR%gen\

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%GEN_DIR%" mkdir "%GEN_DIR%"

PUSHD "%BUILD_DIR%"
call cl /nologo /ZI %DIR%ctype_gen.c
POPD
if ERRORLEVEL 1 EXIT /B 1


echo Generating code...
call %BUILD_DIR%ctype_gen.exe
if ERRORLEVEL 1 EXIT /B 1
echo Done!

ENDLOCAL
EXIT /B 0
