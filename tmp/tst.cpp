#include "sqlw.hpp"

int main()
{
  const char *dbname = "test.db";
  tr::sqlw SqlDb = {dbname};

  if(!SqlDb.ErrorsList.empty())
    for(auto &err: SqlDb.ErrorsList) std::cout << err << "\n";

  SqlDb.open();
  const char *query = "select * from list WHERE year < 1822;";
  SqlDb.exec(query);

  if(!SqlDb.ErrorsList.empty())
    for(auto &err: SqlDb.ErrorsList) std::cout << err << "\n";
  else
    std::cout << SqlDb.num_rows << " rows pass\n";

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
