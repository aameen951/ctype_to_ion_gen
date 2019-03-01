@ECHO off
SETLOCAL

SET DIR=%~dp0

REM ion compiler variables
SET IONHOME=%DIR%..\bitwise\ion
SET ION_COMPILER=%DIR%..\bitwise\ion\build\main.exe

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

echo Compiling test...
call %ION_COMPILER% -o %BUILD_DIR%out_test_gen.c gen
if ERRORLEVEL 1 EXIT /B 1

PUSHD "%BUILD_DIR%"
call cl /nologo /ZI out_test_gen.c
POPD
if ERRORLEVEL 1 EXIT /B 1

echo Running test...
call %BUILD_DIR%out_test_gen.exe
if ERRORLEVEL 1 EXIT /B 1

echo Done!

ENDLOCAL
EXIT /B 0
