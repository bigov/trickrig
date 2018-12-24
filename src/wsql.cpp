/*
 * file: wsql.cpp
 *
 * Обертка для работы с Sqlite3
 *
 */

#include "wsql.hpp"
#include "io.hpp"

namespace tr {

  // Статические члены инициализируются персонально
  //std::forward_list<std::pair<std::string, std::vector<char>>> sqlw::table_row = {};
  std::forward_list<std::forward_list<
    std::pair<std::string, std::vector<char>>>> wsql::Table_rows = {};
  char wsql::empty ='\0';
  int wsql::num_rows = 0;
  tr::query_data wsql::Result = {0, "", "", 0};


  ///
  /// \brief wsql::callback
  /// \param x
  /// \param count
  /// \param value
  /// \param name
  /// \return
  /// \details Обработчик результатов запроса sql3_exec()
  ///
  int wsql::callback(void *x, int count, char **value, char **name)
  {
    std::vector<char> col_value = {};
    std::forward_list<std::pair<std::string, std::vector<char>>> Row = {};

    for(int i = 0; i < count; i++)
    {
      std::string col_name(name[i]);
      col_value.clear();
      if(!value[i])
      { col_value.push_back(empty); }
      else
      {
        size_t k = 0;
        while(value[i][k] != '\0') col_value.push_back(value[i][k++]);
        col_value.push_back(value[i][k]); // завершить массив символом '\0'
      }
      Row.emplace_front(std::make_pair(col_name, col_value));
    }
    Table_rows.push_front(Row);
    num_rows += 1;

    if(nullptr != x) return 1; // В этой реализации значение x всегда равно 0
    else return 0;
  }


  ///
  /// \brief sqlw::request_get
  /// \param Query
  /// \details Обработчик запросов на получение данных
  ///
  void wsql::request_get(const std::string & Query)
  {
    ErrorsList.clear();

    if(Query.empty())
    {
      ErrorsList.emplace_front("Query can't be empty.");
      #ifndef NDEBUG
      for(auto &msg: ErrorsList) tr::info(msg);
      #endif
      return;
    }

    request_get(Query.c_str());
    return;
  }


  ///
  /// \brief sqlw::select_rig
  /// \param x
  /// \param y
  /// \param z
  ///
  void wsql::select_rig(int x, int y, int z)
  {
    char buf[255];
    sprintf(buf,
      "SELECT `born`, `id_area`, `shift` FROM `rigs` "
      "WHERE(`x`=%d AND `y`=%d AND `z`=%d);",
      x, y, z);
    request_get(buf);
    return;
  }


  ///
  /// \brief sqlw::select_snip
  /// \param id
  ///
  void wsql::select_snip(int id)
  {
    char buf[255];
    sprintf(buf, "SELECT `snip` FROM `snips` WHERE `id_area`=%d;", id);
    request_get(buf);
    return;
  }


  /// Запись рига
  void wsql::insert_rig(int x, int y, int z, int t, int id, const float *rs, size_t s)
  {
    char buf[255];
    sprintf(buf, "INSERT OR REPLACE "
                 "INTO `rigs`(`x`,`y`,`z`,`born`,`id_area`, `shift`) "
                 "VALUES(%d, %d, %d, %d, %d, ?);",
                  x, y, z, t, id);
    request_put(buf, rs, s);
    return;
  }


  /// Запись снипа
  void wsql::insert_snip(int id, const float* data)
  {
    char buf[255];
    sprintf(buf, "INSERT INTO `snips`(`id_area`, `snip`) VALUES(%d, ?);",
            id);
    request_put(buf, data, tr::digits_per_snip);
    return;
  }

  /// Обновить номер группы в записи первого снипа
  void wsql::update_snip(int i, int j)
  {
    char buf[255];
    sprintf(buf, "UPDATE `snips` SET `id_area`=%d WHERE `id`=%d;",
            i, j);
    request_put(buf); //TODO: может тут надо exec??
    return;
  }

//////////////////////


  ///
  /// \brief wsql::save_row_data
  /// \details Прием данных, полученых в результате запроса
  ///
  void wsql::save_row_data(void)
  {
    size_t data_bytes = 0;
    std::vector<std::any> Row {};
    std::vector<char> Ch_ceil {};
    std::vector<unsigned char> Uch_ceil {};

    //int col = sqlite3_column_count(pStmt); // Число колонок в результирующем наборе
    int col = sqlite3_data_count(pStmt);     // Число колонок в строке результата

    if(col < 1) return;


    for (int i = 0; i < col; i++)
    {
      int data_type = sqlite3_column_type(pStmt, i);
      Ch_ceil.clear();
      Uch_ceil.clear();

      switch (data_type)
      {
        case SQLITE_INTEGER:
          Row.push_back(std::any(sqlite3_column_int(pStmt, i)));
          break;
        case SQLITE_FLOAT:
          Row.push_back(std::any(sqlite3_column_double(pStmt, i)));
          break;
        case SQLITE_TEXT:
          data_bytes = sqlite3_column_bytes(pStmt, i);
          Ch_ceil.resize(data_bytes + 1, '\0');
          memcpy(Ch_ceil.data(), sqlite3_column_text(pStmt, i), data_bytes);
          Row.push_back(std::any(Ch_ceil));
          break;
        case SQLITE_BLOB:
          data_bytes = sqlite3_column_bytes(pStmt, i);
          Uch_ceil.resize(data_bytes + 1, '\0');
          memcpy(Uch_ceil.data(), sqlite3_column_blob(pStmt, i), data_bytes);
          Row.push_back(std::any(Uch_ceil));
          break;
        case SQLITE_NULL:
          Row.push_back(std::any(NULL));
          break;
        default:
          break;
      }
    }
    Rows.push_front(Row);
    return;
  }


  ///
  /// \brief sqlw::request_get
  /// \param request
  /// \details Обработчик запросов на получение данных
  ///
  void wsql::request_get(const char *request)
  {
    if(!is_open)
    {
#ifndef NDEBUG
      ErrorsList.emplace_front("Not present opened db.");
      for(auto &msg: ErrorsList) tr::info(msg);
#endif
      return;
    }

    bool complete = false;

    Rows.clear();
    ErrorsList.clear();
    if(!request) ErrorsList.emplace_front("query can't be empty.");

    if(SQLITE_OK != sqlite3_prepare_v2(db, request, -1, &pStmt, nullptr))
    {
      ErrorsList.emplace_front("Prepare failed: " + std::string(sqlite3_errmsg(db)));
      complete = true;
    }
    num_rows = 0;
    int step = 0;
    while (!complete)
    {
      step = sqlite3_step(pStmt);
      switch (step)
      {
        case SQLITE_ROW:
          num_rows++;
          save_row_data();
          break;
        case SQLITE_DONE:
          //get_row_data();
          complete = true;
          break;
        case SQLITE_ERROR:
          ErrorsList.emplace_front("Request failed: " + std::string(sqlite3_errmsg(db)));
          complete = true;
          break;
        default:
          ErrorsList.emplace_front("Error: can't exec sqlite3_step(stmt).");
          complete = true;
          break;
      }
    }
    while ((pStmt = sqlite3_next_stmt(db, nullptr)) != nullptr) { sqlite3_finalize(pStmt); }

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) tr::info(msg);
    #endif

    return;
  }


  ///
  /// \brief wsql::request_put
  /// \param Query
  ///
  /// \details Обработчик запросов на сохранение или изменение данных и настроек
  ///
  void wsql::request_put(const std::string & Query)
  {
    request_put(Query.c_str());
    return;
  }


  //## Обработчик запросов на сохранение или изменение данных и настроек
  void wsql::request_put(const char *pzTail)
  {
  /// Может обрабатывать строки, составленые из нескольких запросов,
  /// разделеных стандартным символом (;)
  ///
    if(!is_open)
    {
#ifndef NDEBUG
      ErrorsList.emplace_front("Not present opened db.");
      for(auto &msg: ErrorsList) tr::info(msg);
#endif
      return;
    }

    ErrorsList.clear();

    if(!pzTail)
    {
      ErrorsList.emplace_front("Query can't be empty.");
      return;
    }
    /// Функция sqlite3_prepare_v2 обрабатывает текст (составного) запроса только до
    /// разделителя (;) и возвращает в переменной 'pzTail' адрес начала не обработаной
    /// части полученого текста запроса. Поэтому здесь выполняется цикл до тех пор,
    /// пока все части составного запроса не будут выполнены, а в переменной 'pzTail'
    /// не окажется символ конца строки (empty = '\0').

    const char * request = nullptr;
    while (*pzTail != empty)
    {
      request = pzTail;
      if(SQLITE_OK != sqlite3_prepare_v2(db, request, -1, &pStmt, &pzTail))
      {
        ErrorsList.emplace_front("Prepare failed: " + std::string(sqlite3_errmsg(db)));
        pzTail = &empty;   // аварийный выход
      } else
      {
        if (SQLITE_DONE != sqlite3_step(pStmt))
        {
          ErrorsList.emplace_front("Execution failed: " + std::string(sqlite3_errmsg(db)));
          pzTail = &empty; // аварийный выход
        }
      }
    }

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) tr::info(msg);
    #endif
  }


  ///
  /// \brief Запись бинарных данных, переданых в виде массива и его размера
  ///
  /// \details Данные записываются в соответствии с запросом, текст которого передается
  /// в первом параметре вызова функции. Во втором параметре передаются записываемые
  /// данные, в третьем - их размер в байтах.
  ///
  /// Бинарные данные записываются в поле, обозначенное в тексте запроса символом '?'
  ///
  void wsql::request_put(const char* request, const void* blob_data, size_t blob_size)
  {
    if(!is_open)
    {
#ifndef NDEBUG
      ErrorsList.emplace_front("Not present opened db.");
      for(auto &msg: ErrorsList) tr::info(msg);
#endif
      return;
    }

    ErrorsList.clear();

    if((!request) || (!blob_data) || (0 == blob_size))
      ErrorsList.emplace_front("Query can't be empty.");

    if(SQLITE_OK != sqlite3_prepare_v2(db, request, -1, &pStmt, nullptr))
      ErrorsList.emplace_front("Prepare failed: " + std::string(sqlite3_errmsg(db)));

    // Пятым аргументом можно указать адрес функции деструктора бинарных данных.
    //
    // Если пятым аргументом указать значение SQLITE_STATIC, то SQLite предполагает,
    // что информация (принятые бинарные данные) находится в статическом, неуправляемом
    // пространстве и не нуждается в освобождении.
    //
    // Если пятый аргумент имеет значение SQLITE_TRANSIENT, SQLite делает свою
    // собственную частную копию данных.
    //
    // Как правило, быстрее всего работает вариант с выбором SQLITE_STATIC.

    if (SQLITE_OK != sqlite3_bind_blob(pStmt, 1, blob_data, blob_size, SQLITE_STATIC))
      ErrorsList.emplace_front("Bind failed: " + std::string(sqlite3_errmsg(db)));

    if (SQLITE_DONE != sqlite3_step(pStmt))
      ErrorsList.emplace_front("Execution failed: " + std::string(sqlite3_errmsg(db)));

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) tr::info(msg);
    #endif

  }

  //## Запись бинарных данных переданых в виде массива float и его размера
  void wsql::request_put(const char * request, const float * fl, size_t fl_size)
  {
    size_t data_chars_size = fl_size * sizeof(float);
    std::unique_ptr<char[]> data{new char[data_chars_size]};
    memcpy(data.get(), fl, data_chars_size);
    request_put(request, data.get(), data_chars_size);
    return;
  }

  //## Обработчик запросов вставки/удаления
  void wsql::update_callback( void* udp, int type, const char* db_name,
    const char* tbl_name, sqlite3_int64 rowid )
  {
  /* Функция вызывается при получении запросов на обновление/удаление данных
   * Регистрируется вызовом
   *
   * void* sqlite3_update_hook( sqlite3* db, update_callback, void* udp );
   *
   * udp
   *   An application-defined user-data pointer. This value is made available
   *   to the update callback.
   *
   * type
   *   The type of database update. Possible values are SQLITE_INSERT,
   *   SQLITE_UPDATE, and SQLITE_DELETE.
   *
   * rowid
   *   The ROWID of the row being modified. In the case of an UPDATE, this is
   *   the ROWID value after the modification has taken place.
   */
    if(udp != &empty) ERR("Error in sqlw::updade_callback");
    Result.type = type;
    Result.db_name.clear();
    Result.db_name = db_name;
    Result.tbl_name.clear();
    Result.tbl_name = tbl_name;
    Result.rowid = rowid;
    return;
  }


  ///
  /// \brief wsql::open ## Подключиться к DB
  /// \param fname
  /// \return
  ///
  bool wsql::open(const std::string & FileName)
  {
//DEBUG----------------------------------------------------------------
    info(FileName);
//DEBUG----------------------------------------------------------------

    ErrorsList.clear();
    if(is_open) close(); // закрыть, если был открыт файл

    if(FileName.empty())
    {
      ErrorsList.emplace_front("Sqlw: no specified DB to open.");
      return false;
    }

    int rc = sqlite3_open_v2(FileName.c_str(), &db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK)
    {
      ErrorsList.emplace_front(std::string(sqlite3_errmsg(db))
        + std::string("\nCan't open file: ") + FileName);
      close();
      return false;
    }
    else
    {
      is_open = true;
      sqlite3_update_hook(db, update_callback, &empty);
    }

    return is_open;
  }


  ///
  /// Выполнение запроса
  ///
  void wsql::exec(const std::string &Query)
  {
    exec(Query.c_str());
  }


  ///
  /// \brief sqlw::exec
  /// \param query
  ///
  /// \details Выполнение запроса
  ///
  void wsql::exec(const char *query)
  {
    if(is_open)
    {
      char* err_msg = nullptr;
      ErrorsList.clear();
      Table_rows.clear();
      num_rows = 0;
      if(SQLITE_OK != sqlite3_exec(db, query, callback, 0, &err_msg))
      {
        if(nullptr == err_msg)
        {
          ErrorsList.emplace_front("sqlw::exec: can't exec query "
                                   + std::string(query));
        } else
        {
          ErrorsList.emplace_front("sqlw::exec: " + std::string(err_msg));
          sqlite3_free(err_msg);
        }
      }
    }
    else
    {
      ErrorsList.emplace_front("sqlw::exec: is not opened db-file");
    }

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) info(msg);
    #endif
  }


  ///
  /// \brief wsql::close
  /// \details Закрывает соединение с файлом базы данных
  ///
  void wsql::close(void)
  {
    if(!is_open) return;
    //sqlite3_finalize(pStmt);
    int rc = sqlite3_close(db);

//DEBUG---------------------------------------------------------------------
    if (rc == SQLITE_BUSY) info("SQLite.close: fail");
    else {
      info ("SQLite.close: OK");
    }
    if(!ErrorsList.empty()) for(auto &msg: ErrorsList) info(msg);
//DEBUG---------------------------------------------------------------------


/*
    int rc = sqlite3_close(db);
    if(rc == SQLITE_BUSY)
    {
      sqlite3_stmt * stmt;
      while ((stmt = sqlite3_next_stmt(db, NULL)) != NULL) { sqlite3_finalize(stmt); }
      rc = sqlite3_close(db);
#ifndef NDEBUG
      if (rc != SQLITE_OK) info("Abnormal sqlite3 close");
#endif
    }
*/

    db = nullptr;
    is_open = false;
  }


  ///
  /// \brief wsql::~wsql
  ///
  wsql::~wsql(void)
  {
    if(is_open) close();
  }
}
