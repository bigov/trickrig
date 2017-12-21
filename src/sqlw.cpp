//----------------------------------------------------------------------------
//
// file: sqlw.cpp
//
// Обертка для удобства работы с Sqlite3
//
//----------------------------------------------------------------------------
#include "sqlw.hpp"

namespace tr
{
  // Статические члены инициализируются персонально
  std::forward_list<std::pair<std::string, std::string>> sqlw::row = {};
  std::forward_list<std::forward_list<
    std::pair<std::string, std::string>>> sqlw::rows = {};
  char sqlw::empty ='\0';
  int sqlw::num_rows = 0;

  //## конструктор устанавливает имя файла
  sqlw::sqlw(const char *fname)
  {
    set_db_name(fname);
    return;
  }

  //## конструктор устанавливает имя файла
  sqlw::sqlw(const std::string & fname)
  {
    set_db_name(fname.c_str());
    return;
  }

  //## устанавливает имя файла базы данных Sqlite3
  void sqlw::set_db_name(const std::string & fname)
  {
    set_db_name(fname.c_str());
    return;
  }

  //## устанавливает имя файла базы данных Sqlite3
  void sqlw::set_db_name(const char * fname)
  {
    if(nullptr == fname)
    {
      ErrorsList.emplace_front("Sqlw: no specified DB to open.");
      return;
    }

    db_fname = std::string(fname);
    if(0 == db_fname.length())
      ErrorsList.emplace_front("Sqlw: no specified DB to open.");

    return;
  }

  //## Обработчик результатов запроса
  int sqlw::callback(void *x, int count, char **value, char **name)
  {
    row.clear();
    for(int i = 0; i < count; i++)
    {
      #ifndef NDEBUG
      if (nullptr == name[i]) ERR("sqlw::callback: nullptr name[i]");
      #endif
      row.emplace_front(std::make_pair(name[i], value[i] ? value[i] : &empty));
    }
    rows.push_front(row);
    num_rows++; // у контейнера forward_list нет счетчика элементов

    if(0 != x) return 1; // В этой реализации значение x всегда равно 0
    else return 0;
  }

  //## подключиться к DB
  void sqlw::open(void)
  {
    ErrorsList.clear();
    if(0 != sqlite3_open(db_fname.c_str(), &db))
    {
      ErrorsList.emplace_front("Can't open database: "
        + std::string(sqlite3_errmsg(db))
        + "\nDatabase file name: " + db_fname);
      close();
    } else
    {
      is_open = true;
    }
    return;
  }

  //## Выполнение запроса
  void sqlw::exec(const std::string &query)
  {
    exec(query.c_str());
    return;
  }

  //## Выполнение запроса
  void sqlw::exec(const char *query)
  {
    char *err_msg = nullptr;
    ErrorsList.clear();
    rows.clear();
    num_rows = 0;
    if(SQLITE_OK != sqlite3_exec(db, query, callback, 0, &err_msg))
    {
      ErrorsList.emplace_front("sqlw::exec: " + std::string(err_msg));
      sqlite3_free(err_msg);
    }
    return;
  }

  //## Закрывает соединение с файлом базы данных
  void sqlw::close(void)
  {
    sqlite3_close(db);
    is_open = false;
    return;
  }

  //## Деструктор
  sqlw::~sqlw(void)
  {
    if(is_open) close();
    return;
  }
}
