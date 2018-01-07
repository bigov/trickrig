//----------------------------------------------------------------------------
//
// file: dbw.cpp
//
// Обертка для работы с Sqlite3
//
//----------------------------------------------------------------------------
#include "dbw.hpp"

namespace tr
{
  // Статические члены инициализируются персонально
  std::forward_list<std::pair<std::string, std::vector<char>>> sqlw::row = {};
  std::forward_list<std::forward_list<
    std::pair<std::string, std::vector<char>>>> sqlw::rows = {};
  char sqlw::empty ='\0';
  int sqlw::num_rows = 0;
  tr::query_data sqlw::result = {0, "", "", 0};

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

    DbFileName = std::string(fname);
    if(0 == DbFileName.length())
      ErrorsList.emplace_front("Sqlw: no specified DB to open.");

    return;
  }

  //## Обработчик результатов запроса
  int sqlw::callback(void *x, int count, char **value, char **name)
  {
    std::vector<char> col_value = {};
    row.clear();

    for(int i = 0; i < count; i++)
    {
      std::string col_name(name[i]);
      col_value.clear();
      if(!value[i])
      {  col_value.push_back(empty);  }
      else
      {
        size_t k = 0;
        while(value[i][k] != '\0') col_value.push_back(value[i][k++]);
        col_value.push_back(value[i][k]); // завершить массив символом '\0'
      }
      row.emplace_front(std::make_pair(col_name, col_value));
      //row.push_front(std::make_pair(col_name, col_value));
    }
    rows.push_front(row);
    num_rows++; // у контейнера forward_list нет счетчика элементов

    if(0 != x) return 1; // В этой реализации значение x всегда равно 0
    else return 0;
  }

  //## Обработчик запросов на получение данных
  void sqlw::request_get(const std::string & Query)
  {
    ErrorsList.clear();

    if(Query.empty())
    {
      ErrorsList.emplace_front("Query can't be empty.");
      return;
    }

    ///---///---///TODO

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) tr::info(msg);
    #endif

    return;
  }

  //## Обработчик запросов на сохранение или изменение данных и настроек
  void sqlw::request_put(const std::string & Query)
  {
  /// Может обрабатывать строки, составленые из нескольких запросов,
  /// разделеных стандартным символом (;)

    ErrorsList.clear();

    if(Query.empty())
    {
      ErrorsList.emplace_front("Query can't be empty.");
      return;
    }

    const char *pzTail = Query.c_str();
    const char *request = nullptr;

    /// Функция sqlite3_prepare_v2 обрабатывает текст (составного) запроса только до
    /// разделителя (;) и возвращает в переменной 'pzTail' адрес начала не обработаной
    /// части полученого текста запроса. Поэтому здесь выполняется цикл до тех пор,
    /// пока все части составного запроса не будут выполнены, а в переменной 'pzTail'
    /// не окажется символ конца строки (empty = '\0').

    while (*pzTail != empty)
    {
      request = pzTail;
      if(SQLITE_OK != sqlite3_prepare_v2(db, request, -1, &stmt, &pzTail))
      {
        ErrorsList.emplace_front("Prepare failed: " + std::string(sqlite3_errmsg(db)));
        pzTail = &empty;   // аварийный выход
      } else
      {
        if (SQLITE_DONE != sqlite3_step(stmt))
        {
          ErrorsList.emplace_front("Execution failed: " + std::string(sqlite3_errmsg(db)));
          pzTail = &empty; // аварийный выход
        }
      }
    }

    sqlite3_finalize(stmt);

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) tr::info(msg);
    #endif

    return;
  }

  //## Обработчик запросов вставки/удаления
  void sqlw::update_callback( void* udp, int type, const char* db_name,
    const char* tbl_name, sqlite3_int64 rowid )
  {
  /* Функция вызывается при получении запросов на обновление/удаление данных
   * Регистрируется вызовом
   *
   * void* sqlite3_update_hook( sqlite3* db, update_callback, void* udp );
   *
   * db
   *   A database connection.
   * 
   * update_callback
   *   An application-defined callback function that is called when a database
   *   row is modified.
   * 
   * udp
   *   An application-defined user-data pointer. This value is made available
   *   to the update callback.
   * 
   * type
   *   The type of database update. Possible values are SQLITE_INSERT,
   *   SQLITE_UPDATE, and SQLITE_DELETE.
   * 
   * db_name
   *   The logical name of the database that is being modified. Names include
   *   main, temp, or any name passed to ATTACH DATABASE.
   * 
   * tbl_name
   *   The name of the table that is being modified.
   * 
   * rowid
   *   The ROWID of the row being modified. In the case of an UPDATE, this is
   *   the ROWID value after the modification has taken place.
   * 
   * Returns (sqlite3_update_hook()) - the previous user-data pointer,
   * if applicable.
   * 
   */
    if(udp != &empty) ERR("Error in sqlw::updade_callback");
    
    result.type = type;
    
    result.db_name.clear();
    result.db_name = db_name;
    
    result.tbl_name.clear();
    result.tbl_name = tbl_name;
    
    result.rowid = rowid;
    
    return;
  }
  
  //## Подключиться к DB
  void sqlw::open(const std::string & fname)
  {
  /* Закрывает текущий файл БД (если был открыт) и открывает новый
   */
    if(is_open) close();
    DbFileName = fname;
    open();
    return;
  }

  //## Подключиться к DB
  void sqlw::open(void)
  {
    if(DbFileName.empty())
    {
      ErrorsList.emplace_front("Sqlw: no specified DB to open.");
      return;
    }

    ErrorsList.clear();

    int rc = sqlite3_open_v2(DbFileName.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
      ErrorsList.emplace_front("Can't open database: "
        + std::string(sqlite3_errmsg(db))
        + "\nDatabase file name: " + DbFileName);
      close();
    } else
    {
      is_open = true;
      sqlite3_update_hook(db, update_callback, &empty);
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
    if(!is_open) open();
    char *err_msg = nullptr;
    ErrorsList.clear();
    rows.clear();
    num_rows = 0;
    if(SQLITE_OK != sqlite3_exec(db, query, callback, 0, &err_msg))
    {
      ErrorsList.emplace_front("sqlw::exec: " + std::string(err_msg));
      sqlite3_free(err_msg);
    }

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) tr::info(msg);
    #endif

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
