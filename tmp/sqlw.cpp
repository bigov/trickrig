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
  void info(const char* msg)
  {
    std::cout << msg << "\n";
    return;
  }

  void info(const std::string & msg)
  {
    info(msg.c_str());
    return;
  }

  // Статические члены инициализируются персонально
  std::forward_list<std::forward_list<
    std::pair<std::string, std::string>>> sqlw::rows = {};

  //## конструктор только устанавливает имя файла
  sqlw::sqlw(const char *fname)
  {
  /* Для обеспечения возможности открывать и закрывать соединения в
   * соответствии с логикой работы приложения конструктор по-умолчанию не
   * открывает соединение.
   */
    db_fname = std::string(fname);
    if(0 == db_fname.length()) tr::info("sqlw::sqlw don't get database name.");
    return;
  }

  //## Обработчик результатов запроса
  int sqlw::callback(void *NotUsed, int argc, char **argv, char **azColName)
  {
    std::string c_name, c_date;
    std::forward_list<std::pair<std::string, std::string>> row;

    int i;
    for(i=0; i<argc; i++){
      c_name = std::string(azColName[i]);
      if(nullptr != argv[i]) c_date = std::string(argv[i]);
      else c_date = "";
      row.push_front(std::make_pair(c_name, c_date));
    }
    rows.push_front(row);
    return 0;
  }

  //## подключиться к DB
  void sqlw::open(void)
  {
    if(0 != sqlite3_open(db_fname.c_str(), &db))
    {
      tr::info("Can't open database: " + std::string(sqlite3_errmsg(db))
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
    if(SQLITE_OK != sqlite3_exec(db, query, callback, 0, &err_msg))
    {
      tr::info("sqlw::exec :" + std::string(err_msg));
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
