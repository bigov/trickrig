::
:: Ключи командной строки:
::
:: clean - полная очистка и пересборка
:: init - создание Makefile без сборки бинарника
::

@ECHO OFF
PUSHD
SETLOCAL

SET "BAKE_DIR=_dbg_"
SET "CMAKE=C:\usr\bin\cmake.exe"
SET "PATH=..\dlls;C:\usr\bin;C:\Windows\system32;C:\Windows"
SET "CC=C:/usr/bin/clang"
SET "CXX=C:/usr/bin/clang++"

IF EXIST CMakeLists.txt (
	MKDIR %BAKE_DIR%
	COPY /Y /L bake.cmd %BAKE_DIR%\
	CD %BAKE_DIR%
	ECHO.
	ECHO ВНИМАНИЕ! Для сборки используйте папку '%BAKE_DIR%'.
	ECHO.
	CALL bake.cmd
	GOTO _eof
)

IF "%1"=="clean" (
	RD /Q /S CMakeFiles
	DEL /Q *make*.*
	DEL /Q app*.exe
)

"%CMAKE%" -DCMAKE_BUILD_TYPE=Debug ..\ -G "MinGW Makefiles"

IF "%1"=="init" GOTO _eof
IF "%2"=="init" GOTO _eof

"%CMAKE%" --build .

IF ERRORLEVEL 1 (
	ECHO ----------------
	ECHO ..ошибка сборки
	ECHO.
	pause
	GOTO _eof
)

CALL app_dbg.exe
IF ERRORLEVEL 1 pause

:_eof
ECHO.
ENDLOCAL
POPD

