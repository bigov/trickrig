//----------------------------------------------------------------------------
//
// file: sqlw.hpp
//
// Обертка для удобства работы с Sqlite3
//
//----------------------------------------------------------------------------
#ifndef __SQLW_HPP__
#define __SQLW_HPP__

#include "main.hpp"

namespace tr{

class sqlw
{
  public:
    sqlw(void) { DbFileName.clear(); }
    sqlw(const char *);
    sqlw(const std::string &);
    ~sqlw(void);

    static std::forward_list<std::pair<std::string, std::string>> row;
    static std::forward_list<
           std::forward_list<std::pair<std::string, std::string>>> rows;
    static int num_rows; // число строк в результате запроса

    std::forward_list<std::string> ErrorsList = {};

    void set_db_name(const char *);
    void set_db_name(const std::string &);
    void open(void);
    void open(const std::string &);
    void close(void);
    void exec(const char *);
    void exec(const std::string &);

  private:
    static char empty;

    sqlw(const sqlw &) = delete;
    sqlw& operator=(const sqlw &) = delete;

    sqlite3 *db = nullptr;
    bool is_open = false;
    std::string DbFileName = "";

    static int callback(void*, int, char**, char**);

}; // class sqlw
}  // ns tr::
#endif
