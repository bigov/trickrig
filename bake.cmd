::
:: Ключи командной строки:
::
:: clean - полная очистка и пересборка
:: init - создание Makefile без сборки бинарника
::

@ECHO OFF
PUSHD
SETLOCAL
SET "WD=_dbg_"

:: Если для сборки использовать clang
IF "%1"=="clang" (
	SET "CC=C:/usr/bin/clang"
	SET "CXX=C:/usr/bin/clang++"
)

IF EXIST CMakeLists.txt (
	MKDIR %WD%
	COPY /Y /L bake.cmd %WD%\
	CD %WD%
	ECHO.
	ECHO ВНИМАНИЕ! Для сборки используйте папку '%WD%'.
	ECHO.
	CALL bake.cmd
	GOTO _eof
)

IF "%1"=="clean" (
	RD /Q /S CMakeFiles
	DEL /Q *make*.*
	DEL /Q app*.exe
)

cmake -DCMAKE_BUILD_TYPE=Debug ..\ -G "MinGW Makefiles"

IF "%1"=="init" GOTO _eof
IF "%2"=="init" GOTO _eof

cmake --build .

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
