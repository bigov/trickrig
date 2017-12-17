/*
  Пример из краткого введения в API Sqlite на странице документации
  http://www.sqlite.org/quickstart.html

Тестовая база данных: test.db содержит одну таблицу:

tst_CPP_dev>sqlite3.exe test.db
SQLite version 3.21.0 2017-10-24 18:55:49
Enter ".help" for usage hints.
sqlite> create table list (
   ...> rowid INTEGER PRIMARY KEY,
   ...> event TEXT,
   ...> year INTEGER(2));
sqlite> insert into list(event,year) values('now year is', 2017);
sqlite> select * from list;
1|now year is|2017
sqlite> .quit
tst_CPP_dev>

*/
#include <stdio.h>
#include "sqlite3.h"

//## Вывод результата запроса к базе данных
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

//## == ВХОД ==
int main(int argc, char **argv)
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  if( argc!=3 ){
    fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
    return(1);
  }
  rc = sqlite3_open(argv[1], &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }
  rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  sqlite3_close(db);
  return 0;
}
/***

 Компилируется в исполняемый файл одной командой:

 tst_CPP_dev>gcc sample.cpp lib\sqlite3.c -o sample

 Результат проверяется просто:

 tst_CPP_dev>sample test.db "SELECT * FROM list;"
rowid = 1
event = now year is
year = 2017

*/