//----------------------------------------------------------------------------
//
// file: dbw.hpp
//
// Обертка для работы с Sqlite3
//
//----------------------------------------------------------------------------
#ifndef __DBW_HPP__
#define __DBW_HPP__

#include "main.hpp"
#include "io.hpp"
#include "sqlite3.h"

namespace tr{
  
struct query_data {
  int type;
  std::string db_name;
  std::string tbl_name;
  sqlite3_int64 rowid;
};

class sqlw
{
  public:
    sqlw(void) { DbFileName.clear(); }
    sqlw(const char *);
    sqlw(const std::string &);
    ~sqlw(void);

    // Результат выполнения запроса "exec" записывается парами заголовок_поля:значение
    static std::forward_list<
           std::forward_list<std::pair<std::string, std::vector<char>>>> Table_rows;
    static int num_rows; // число строк в результате запроса

    // Результат выполнения запроса "request_get"
    std::forward_list<std::vector<std::any>> Rows;

    std::forward_list<std::string> ErrorsList = {};
    static tr::query_data Result;

    void set_db_name(const char *);
    void set_db_name(const std::string &);
    void open(void);
    void open(const std::string &);
    void close(void);
    void exec(const char *);
    void exec(const std::string &);
    void request_put(const char *);
    void request_put(const char *, const char *, size_t);
    void request_put(const char *, const float *, size_t);
    void request_put(const std::string &);

    void request_get(const char *);
    void request_get(const std::string &);

    const char *tpl_select_rig =
      "SELECT `born`, `id_area`, `shift` FROM `rigs` WHERE(`x`=%d AND `y`=%d AND `z`=%d);%c";
    const char *tpl_select_snip =
      "SELECT `snip` FROM `snips` WHERE `id_area`=%d;%c";
    const char *tpl_insert_rig =
      "INSERT OR REPLACE INTO `rigs`(`x`,`y`,`z`,`born`,`id_area`, `shift`) VALUES(%d, %d, %d, %d, %d, ?);%c";
    const char *tpl_insert_snip =
      "INSERT INTO `snips`(`id_area`, `snip`) VALUES(%d, ?);%c";
    const char *tpl_update_snip =
      "UPDATE `snips` SET `id_area`=%d WHERE `id`=%d;%c";

  private:
    static char empty;

    sqlw(const sqlw &) = delete;
    sqlw& operator=(const sqlw &) = delete;

    sqlite3 *db = nullptr;
    sqlite3_stmt *pStmt = nullptr;

    bool is_open = false;
    std::string DbFileName = "";

    void save_row_data(void);
    static int callback(void*, int, char**, char**);
    static void update_callback(void*, int, const char*, const char*,
      sqlite3_int64);

};     // class sqlw
}      // ns tr::
#endif // __DBW_HPP__
