//----------------------------------------------------------------------------
//
// file: sqlw.hpp
//
// Обертка для удобства работы с Sqlite3
//
//----------------------------------------------------------------------------
#ifndef __SQLW_HPP__
#define __SQLW_HPP__

#include <iostream>
#include <forward_list>
#include <string>
#include <stdio.h>
#include "../.extlibs/sqlite3/sqlite3.h"

namespace tr{

class sqlw
{
  public:
    sqlw(const char *);
    ~sqlw(void);

    static std::forward_list<
      std::forward_list<
        std::pair<std::string, std::string>>> rows;

    void open(void);
    void close(void);
    void exec(const char *query);
    void exec(const std::string &query);

  private:
    sqlw(const sqlw &) = delete;
    sqlw& operator=(const sqlw &) = delete;

    sqlite3 *db = nullptr;
    bool is_open = false;
    std::string db_fname = "";

    static int callback(void*, int, char**, char**);

}; // class sqlw
}  // ns tr::
#endif
