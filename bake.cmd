::
:: Ключи командной строки:
::
:: clang - компиляция программой clang
:: clang release - компиляция программой clang без отладки
:: gcc release   - компиляция программой gcc без отладки
::
:: clean - полная очистка и пересборка
:: init - создание Makefile без сборки бинарника
::

@ECHO OFF
PUSHD
SETLOCAL
SET "WD=_dbg_"

SET "n2=dbg"
SET "n3=gcc"

:: Если для сборки использовать clang
IF "%1"=="clang" (
	SET "CC=f:/usr/bin/clang.exe"
	SET "CXX=f:/usr/bin/clang++.exe"
	SET "n3=clang"
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
	GOTO _eof
)

IF "%2"=="release" (
	cmake -DCMAKE_BUILD_TYPE=Release ..\ -G "MinGW Makefiles"
	SET "n2=rel"
) ELSE (
	cmake -DCMAKE_BUILD_TYPE=Debug ..\ -G "MinGW Makefiles"
)

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

CALL app_%n2%_%n3%.exe
IF ERRORLEVEL 1 pause

:_eof
ECHO.
ENDLOCAL
POPD
