::
:: Генерация файлов тегов и настройки VIM
::
@ECHO OFF
PUSHD
SETLOCAL

:: текущая папка
SET "DP=%~dp0"

SET "TAGS=ctags -a -u -R --languages=C,C++ --C++-kinds=+pl --fields=+iaS --extra=+fq"

ECHO.
DEL /F /Q tags
::ECHO Creating C++ tags files from dirs:
::ECHO.

::@ECHO ON
call %TAGS% %DP%src
call %TAGS% %DP%include
call %TAGS% %DP%lib
@ECHO OFF

ENDLOCAL
POPD

ECHO Update tags:
COPY /A /Y tags src\
COPY /A /Y tags include\
ECHO Complete.
ECHO.
ECHO.

