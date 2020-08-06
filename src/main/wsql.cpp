/*
 * file: wsql.cpp
 *
 * Обертка для работы с Sqlite3
 *
 */

#include "wsql.hpp"
#include "tools.hpp"

namespace tr {

  // Статические члены инициализируются персонально
  //std::forward_list<std::pair<std::string, std::vector<char>>> sqlw::table_row = {};
  std::forward_list<std::forward_list<
    std::pair<std::string, std::vector<char>>>> wsql::Table_rows = {};
  char wsql::empty ='\0';
  int wsql::num_rows = 0;
  tr::query_data wsql::Result = {0, "", "", 0};

  ///
  /// \brief loadOrSaveDb
  /// \param pInMemory
  /// \param zFilename
  /// \param isSave
  /// \return
  /// \details   This function is used to load the contents of a database file on disk
  /// into the "main" database of open database connection pInMemory, or
  /// to save the current contents of the database opened by pInMemory into
  /// a database file on disk. pInMemory is probably an in-memory database,
  /// but this function will also work fine if it is not.
  ///
  /// Parameter zFilename points to a nul-terminated string containing the
  /// name of the database file on disk to load from or save to. If parameter
  /// isSave is non-zero, then the contents of the file zFilename are
  /// overwritten with the contents of the database opened by pInMemory. If
  /// parameter isSave is zero, then the contents of the database opened by
  /// pInMemory are replaced by data loaded from the file zFilename.
  ///
  /// If the operation is successful, SQLITE_OK is returned. Otherwise, if
  /// an error occurs, an SQLite error code is returned.
  ///
  int wsql::loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave)
  {
    int rc;                   /* Function return code */
    sqlite3 *pFile;           /* Database connection opened on zFilename */

    /* Open the database file identified by zFilename. Exit early if this fails
    ** for any reason. */
    rc = sqlite3_open(zFilename, &pFile);
    if( rc==SQLITE_OK ){

      /* If this is a 'load' operation (isSave==0), then data is copied
      ** from the database file just opened to database pInMemory.
      ** Otherwise, if this is a 'save' operation (isSave==1), then data
      ** is copied from pInMemory to pFile.  Set the variables pFrom and
      ** pTo accordingly. */
      sqlite3* pFrom = (isSave ? pInMemory : pFile);
      sqlite3* pTo   = (isSave ? pFile     : pInMemory);

      /* Set up the backup procedure to copy from the "main" database of
      ** connection pFile to the main database of connection pInMemory.
      ** If something goes wrong, pBackup will be set to NULL and an error
      ** code and message left in connection pTo.
      **
      ** If the backup object is successfully created, call backup_step()
      ** to copy data from pFile to pInMemory. Then call backup_finish()
      ** to release resources associated with the pBackup object.  If an
      ** error occurred, then an error code and message will be left in
      ** connection pTo. If no error occurred, then the error code belonging
      ** to pTo is set to SQLITE_OK.
      */
      sqlite3_backup* pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
      if( pBackup ){
        (void)sqlite3_backup_step(pBackup, -1);
        (void)sqlite3_backup_finish(pBackup);
      }
      rc = sqlite3_errcode(pTo);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void)sqlite3_close(pFile);
    return rc;
  }


  ///
  /// \brief wsql::open_in_ram
  /// \return
  /// \details Загружает данные из файла в базу данных, размещенную
  /// в оперативной памяти
  ///
  bool wsql::open_in_ram(const std::string& FileNameDB)
  {
    assert(!is_open);
    ErrorsList.clear();

    int rc = sqlite3_open_v2(":memory:", &db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK)
    {
      ErrorsList.emplace_front(std::string(sqlite3_errmsg(db))
        + std::string("\nCan't open :memory: database."));
      if(db != nullptr) sqlite3_close(db);
      return false;
    }
    else
    {
      sqlite3_update_hook(db, update_callback, &empty);
      rc = loadOrSaveDb(db, FileNameDB.c_str(), 0);
      is_open = (rc == SQLITE_OK);
    }
    return is_open;
  }


  ///
  /// \brief wsql::close_in_ram
  /// \param FileNameDB
  /// \details закрывает базу данных в памяти с сохранением данных в файл на диске.
  ///
  void wsql::close_in_ram(const std::string& FileNameDB)
  {
    loadOrSaveDb(db, FileNameDB.c_str(), 1);
    close();
  }


  ///
  /// \brief wsql::callback
  /// \param x
  /// \param count
  /// \param value
  /// \param name
  /// \return
  /// \details Обработчик результатов запроса sql3_exec()
  ///
  /// !ВНИМАНИЕ! НЕ ИСПОЛЬЗОВАТЬ ДЛЯ ПОЛУЧЕНИЯ ДАННЫХ ИЗ ПОЛЕЙ ТИПА BLOB
  ///
  int wsql::callback(void *x, int count, char **value, char **name)
  {
    std::vector<char> col_value {};
    std::forward_list<std::pair<std::string, std::vector<char>>> Row {};

    for(int i = 0; i < count; i++)
    {
      std::string col_name(name[i]);
      col_value.clear();
      if(!value[i])
      { col_value.push_back(empty); }
      else
      {
        size_t k = 0;

        // Если поле типа BLOB и есть значения '\0', то они все будут потеряны.
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
  /// \brief wsql::save_row_data
  /// \details Прием данных, полученых в результате запроса
  ///
  void wsql::save_row_data(void)
  {
    size_t lgth = 0;
    std::list<std::vector<uchar>> RowData {}; // Строка ячеек таблицы результата запроса

    int col = sqlite3_data_count(pStmt);     // Число колонок в строке результата
    if(col < 1) return;

    for (int n = 0; n < col; n++)
    {
      lgth = sqlite3_column_bytes(pStmt, n);    // размер ячейки результата в байтах
      std::vector<uchar> CeilData(lgth, 0);  // массив для приема данных

      int i; double d;
      switch (sqlite3_column_type(pStmt, n))
      {
        case SQLITE_INTEGER:
          i = sqlite3_column_int(pStmt, n);
          memcpy(CeilData.data(), &i, lgth);
          break;
        case SQLITE_FLOAT:
          d = sqlite3_column_double(pStmt, n);
          memcpy(CeilData.data(), &d, lgth);
          break;
        case SQLITE_TEXT:
          memcpy(CeilData.data(), sqlite3_column_text(pStmt, n), lgth);
          break;
        case SQLITE_BLOB:
          memcpy(CeilData.data(), sqlite3_column_blob(pStmt, n), lgth);
          break;
        case SQLITE_NULL: default:
          break;
      }
      RowData.push_back(std::move(CeilData));
    }
    Rows.push_front(std::move(RowData));
    return;
  }


///
/// \brief sqlw::request_get
/// \param request
/// \details Обработчик запросов на получение данных
///
std::vector<unsigned char> wsql::request_get(const char *request)
{
#ifndef NDEBUG
  assert(is_open);
#endif

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
  while (!complete)
  {
    switch (sqlite3_step(pStmt))
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
  for(auto &msg: ErrorsList) std::cerr << msg << std::endl;
  #endif

  if(Rows.empty()) return std::vector<unsigned char> {};
  return Rows.front().back();
}


  ///
  /// \brief wsql::request_put
  /// \param pzTail
  ///
  /// \details Обработчик запросов на сохранение или изменение данных и настроек
  ///
  /// Может обрабатывать строки, составленые из нескольких запросов,
  /// разделеных стандартным символом (;)
  ///
  /// Функция sqlite3_prepare_v2 обрабатывает текст (составного) запроса только до
  /// разделителя (;) и возвращает в переменной 'pzTail' адрес начала не обработаной
  /// части полученого текста запроса. Поэтому здесь выполняется цикл до тех пор,
  /// пока все части составного запроса не будут выполнены, а в переменной 'pzTail'
  /// не окажется символ конца строки (empty = '\0').
  ///
  void wsql::write(const char *pzTail)
  {
    assert(is_open);

    ErrorsList.clear();

    if(!pzTail)
    {
      ErrorsList.emplace_front("Query can't be empty.");
      return;
    }

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

    while ((pStmt = sqlite3_next_stmt(db, nullptr)) != nullptr) { sqlite3_finalize(pStmt); }

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) std::cerr << msg << std::endl;
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
  /// Функция "sqlite3_bind_blob"
  /// ---------------------------
  /// Четвертый параметр - это размер массива в байтах
  ///
  /// Пятым аргументом можно указать адрес функции деструктора бинарных данных. Если
  /// пятым аргументом указать значение SQLITE_STATIC, то SQLite предполагает, что
  /// информация (принятые бинарные данные) находится в статическом, неуправляемом
  /// пространстве и не нуждается в освобождении.
  ///
  /// Если пятый аргумент имеет значение SQLITE_TRANSIENT, SQLite делает свою
  /// собственную частную копию данных.
  ///
  /// Как правило, быстрее всего работает вариант с выбором SQLITE_STATIC.
  ///
  void wsql::request_put(const char* request, const void* blob_data, int blob_size)
  {
    assert(is_open);

    ErrorsList.clear();

    if((!request) || (!blob_data) || (0 == blob_size))
      ErrorsList.emplace_front("Query can't be empty.");

    if(SQLITE_OK != sqlite3_prepare_v2(db, request, -1, &pStmt, nullptr))
      ErrorsList.emplace_front("Prepare failed: " + std::string(sqlite3_errmsg(db)));

    if (SQLITE_OK != sqlite3_bind_blob(pStmt, 1, blob_data, blob_size, SQLITE_STATIC))
      ErrorsList.emplace_front("Bind failed: " + std::string(sqlite3_errmsg(db)));

    if (SQLITE_DONE != sqlite3_step(pStmt))
      ErrorsList.emplace_front("Execution failed: " + std::string(sqlite3_errmsg(db)));

    sqlite3_finalize(pStmt);

    #ifndef NDEBUG
    for(auto &msg: ErrorsList) std::cerr << msg << std::endl;
    #endif
  }


  ///
  /// \brief wsql::request_put_float
  /// \param request
  /// \param fl
  /// \param fl_size
  /// \details Запись бинарных данных переданых в виде массива float и его размера
  ///
  void wsql::request_put_float(const char* request, const float* fl, size_t fl_size)
  {
    auto data_chars_size = fl_size * sizeof(float);
    std::unique_ptr<char[]> data {new char[data_chars_size]};
    memcpy(data.get(), fl, data_chars_size);
    request_put(request, data.get(), static_cast<int>(data_chars_size));
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
    assert(!is_open && "Trying to reopen the database file");
    ErrorsList.clear();

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
      if(db != nullptr) sqlite3_close(db);
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
  /// \brief sqlw::exec
  /// \param query
  ///
  /// \details Выполнение запроса
  ///
  /// !ВНИМАНИЕ! НЕ УМЕЕТ ОБРАБАТЫВАТЬ ДАННЫЕ ТИПА BLOB
  ///
  void wsql::exec(const char* query)
  {
#ifndef NDEBUG
  assert(is_open);
#endif

    char* err_msg = nullptr;
    ErrorsList.clear();
    Table_rows.clear();
    num_rows = 0;
    if(SQLITE_OK != sqlite3_exec(db, query, callback, nullptr, &err_msg))
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

#ifndef NDEBUG
  for(auto& msg: ErrorsList) std::cerr << msg << std::endl;
#endif
  }


  ///
  /// \brief wsql::close
  /// \details Закрывает соединение с файлом базы данных
  ///
  void wsql::close(void)
  {
    assert(is_open);

    #ifndef NDEBUG
    if (sqlite3_close(db) == SQLITE_BUSY) std::cerr << "SQLite.close: fail" << std::endl;
    #else
    sqlite3_close(db);
    #endif

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
