#include "sqlw.hpp"

int main()
{
  const char *dbname = "test.db";
  tr::sqlw SqlDb = {dbname};
  SqlDb.open();
  const char *query = "select * from list;";
  SqlDb.exec(query);

  for(auto &row: SqlDb.rows)
  {
    for(auto &v: row)
    {
      if(v.second.empty()) std::cout << "--- --- value is empty:\n";
      std::cout << v.first << ": " << v.second << "\n";
    }
  }
  std::cout << "\n";

  SqlDb.close();
  return 0;
}
