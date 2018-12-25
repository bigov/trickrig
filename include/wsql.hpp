/*
 * file: wsql.hpp
 *
 * Заголовки класса обертки к sqlite3
 *
 */

#ifndef WSQL_HPP
#define WSQL_HPP

#include "main.hpp"
#include "sqlite3.h"

namespace tr{
  
struct query_data {
  int type;
  std::string db_name;
  std::string tbl_name;
  sqlite3_int64 rowid;
};

class wsql
{
  public:
    wsql(void) { }
    ~wsql(void);

    // Результат выполнения запроса "exec" записывается парами заголовок_поля:значение
    static std::forward_list<
           std::forward_list<std::pair<std::string, std::vector<char>>>> Table_rows;
    static int num_rows; // число строк в результате запроса

    // Результат выполнения запроса "request_get"
    std::forward_list<std::vector<std::any>> Rows;

    std::forward_list<std::string> ErrorsList = {};
    static tr::query_data Result;

    bool open(const std::string &);
    void close(void);
    void exec(const char *);
    void write(const char *);
    void request_put(const char *, const void *, size_t);
    void request_put(const char *, const float *, size_t);
    void request_get(const char *);

    void select_rig(int, int, int);
    void select_snip(int);
    void insert_rig(int, int, int, int, int, const float *, size_t);
    void insert_snip(int, const float *);
    void update_snip(int, int);

  private:
    wsql(const wsql &) = delete;
    wsql& operator=(const wsql &) = delete;

    static char empty;
    sqlite3 *db = nullptr;
    sqlite3_stmt *pStmt = nullptr;

    bool is_open = false;

    bool _open(void);
    void save_row_data(void);
    static int callback(void*, int, char**, char**);
    static void update_callback(void*, int, const char*, const char*, sqlite3_int64);

};     // class sqlw
}      // ns tr::

#endif //WSQL_HPP

