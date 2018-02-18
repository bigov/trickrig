@ECHO OFF
IF EXIST bake.cmd GOTO _err
rd /S /Q CMakeFiles && del /S /Q /F *
cls
cmake ../ -G "MinGW Makefiles"
cmake --build . && server.exe
ECHO.
pause

GOTO _eof
:_err
ECHO "ERROR: run from external directory"
pause
:_eof
