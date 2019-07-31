/*
 * file: db.cpp
 *
 * Управление базой данных
 *
 *

Схема базы данных поверхности
=============================

CREATE TABLE "rigs" (
  `id`      INTEGER NOT NULL UNIQUE,
  `x`       INTEGER,
  `y`       INTEGER,
  `z`       INTEGER,
  `born`    INTEGER,
  `id_area` INTEGER NOT NULL, PRIMARY KEY(`id`) );

CREATE TABLE "snips" (
  `id`      INTEGER NOT NULL UNIQUE,
  `snip`    BLOB NOT NULL,
  `id_area` INTEGER NOT NULL, PRIMARY KEY(`id`) );

CREATE INDEX `area_on_rigs` ON `rigs` ( `id_area` );

CREATE INDEX `area_on_snips` ON `snips` ( `id_area` );

CREATE UNIQUE INDEX `c3d` ON `rigs` ( `x`, `y`, `z` );

 *
 */

#include "db.hpp"

namespace tr {


///
/// \brief db::load_config
/// \param n
/// \param Pname
/// \return
///
/// \details Общий загрузчик данных из файлов конфигурации приложения и карты
///
v_str db::load_config(size_t n, const std::string &Pname)
{
  v_str ConfigParams {};
  ConfigParams.resize(n);
  if(!SqlDb.open(Pname)) ERR("Can't open DB file: " + Pname);
  SqlDb.exec("SELECT * FROM init;");

  if(SqlDb.num_rows < 1)
  {
    std::string IsFail = "Fail loading config.\n";
    for(auto &err: SqlDb.ErrorsList) IsFail += err + "\n";
    SqlDb.close();
    ERR(IsFail);
  }

  // Данные из таблицы результата переносим в массив ConfigParams
  size_t key = 0;
  std::string val {};

  for(auto &row: SqlDb.Table_rows)
  {
    for(auto &p: row)
    {
      if(0 == p.first.find("key")) key = static_cast<size_t>(std::stoi(p.second.data()));
      if(0 == p.first.find("val")) val = std::string(p.second.data());
    }
    ConfigParams[key] = val;
  }
  SqlDb.close();

  return ConfigParams;
}


///
/// \brief db::get_map_name
/// \param dbFile
/// \return
/// \details Возвращает значение имени карты из конфига
///
v_ch db::map_name_read(const std::string & dbFile)
{
  v_ch result {}; result.clear();

  SqlDb.open(dbFile);
  std::string Query =
      "SELECT val from init WHERE key="+std::to_string(MAP_NAME)+";";

  SqlDb.exec(Query.c_str());
  if(!SqlDb.Table_rows.empty()) result = SqlDb.Table_rows.front().front().second;

  SqlDb.close();
  return result;
}


///
/// \brief db::get_voxel
/// \param i3d P
/// \param size
/// \return
///
/// \details Если в базе данных по указанным координатам найдены параметры
/// вокса, то в оперативной памяти создается объект с полученными
/// параметрами и возвращается уникальный указатель на него.
///
std::unique_ptr<vox> db::get_vox(const i3d& P, int size)
{
  char query[255];
  sprintf(query, "SELECT * FROM voxels WHERE x=%d AND y=%d AND z=%d AND size=%d;",
          P.x, P.y, P.z, size);
  SqlDb.exec(query);

  if(!SqlDb.Table_rows.empty())
    return std::make_unique<vox>(P, size);
  else
    return nullptr;
}



///
/// \brief db::open
/// \param PathName - путь к директории данных карты пользователя (cо слэшем в конце)
///
v_str db::map_open(const std::string &DirPathName)
{
  CfgMapPFName = DirPathName + fname_cfg;
  MapPFName = DirPathName + fname_map;
  if(!fs::exists(CfgMapPFName)) init_map_config(CfgMapPFName);
  v_str Result = load_config(MAP_INIT_SIZE, CfgMapPFName);

  // Перед выходом из функции чтения параметров карты открыть файл данных карты.
  // Закроется файл при вызове функции сохранения конфига.
  SqlDb.open_in_ram(MapPFName);

  return Result;

}


///
/// \brief db::open
/// \param PathName - путь к директории данных приложения (cо слэшем в конце)
///
v_str db::open_app(const std::string &DirPathName)
{
  CfgAppPFName = DirPathName + fname_cfg;
  if(!fs::exists(CfgAppPFName)) init_app_config(CfgAppPFName);
  return load_config(APP_INIT_SIZE, CfgAppPFName);
}


///
/// \brief db::save_map_name
/// \param Dir      имя папки со слешем на конце
/// \param MapName  имя карты
///
void db::map_name_save(const std::string &Dir, const std::string &MapName)
{
  // Записать в конфиг имя карты, введенное пользователем
  std::string Query = "INSERT INTO init (key, val) VALUES ("+
      std::to_string(MAP_NAME) +", \""+ MapName.c_str() +"\");";
  SqlDb.open(Dir + fname_cfg);
  SqlDb.exec(Query.c_str());
  SqlDb.close();
}


///
/// \brief db::save
/// \param Eye
///
void db::map_close(const tr::camera_3d &Eye)
{
  SqlDb.close_in_ram(MapPFName); // закрыть файл данных пространства вокселей

  char q [255]; // буфер для форматирования и передачи строки в запрос
  const char tpl[] = "UPDATE init SET val='%s' WHERE key=%d;";
  std::string p = "";
  std::string Query = "";

  p = std::to_string(Eye.ViewFrom.x);
  sprintf(q, tpl, p.c_str(), VIEW_FROM_X);
  Query += q;

  p = std::to_string(Eye.ViewFrom.y);
  sprintf(q, tpl, p.c_str(), VIEW_FROM_Y);
  Query += q;

  p = std::to_string(Eye.ViewFrom.z);
  sprintf(q, tpl, p.c_str(), VIEW_FROM_Z);
  Query += q;

  p = std::to_string(Eye.look_a);
  sprintf(q, tpl, p.c_str(), LOOK_AZIM);
  Query += q;

  p = std::to_string(Eye.look_t);
  sprintf(q, tpl, p.c_str(), LOOK_TANG);
  Query += q;

  if(SqlDb.open(CfgMapPFName))
  {
    SqlDb.write(Query.c_str());
    SqlDb.close();
  }
}


///
/// \brief db::save_vox
/// \param V
///
void db::save_vox(vox* V)
{
  char q [255]; // буфер для форматирования и передачи строки в запрос
  sprintf(q, "INSERT INTO voxels (x, y, z, size) VALUES (%d, %d, %d, %d);",
          V->Origin.x, V->Origin.y, V->Origin.z, V->side_len);
  SqlDb.write(q);
}


///
/// \brief db::save
/// \param AppWin
///
void db::save_window_params(const tr::main_window &AppWin)
{
  char q [255]; // буфер для форматирования и передачи строки в запрос
  const char tpl[] = "UPDATE init SET val='%s' WHERE key=%d;";
  std::string p = "";
  std::string Query = "";

  p = std::to_string(AppWin.left);
  sprintf(q, tpl, p.c_str(), WINDOW_LEFT);
  Query += q;

  p = std::to_string(AppWin.top);
  sprintf(q, tpl, p.c_str(), WINDOW_TOP);
  Query += q;

  p = std::to_string(AppWin.width);
  sprintf(q, tpl, p.c_str(), WINDOW_WIDTH);
  Query += q;

  p = std::to_string(AppWin.height);
  sprintf(q, tpl, p.c_str(), WINDOW_HEIGHT);
  Query += q;

  if(!SqlDb.open(CfgAppPFName)) ERR("Fail: not found app config.");
  SqlDb.write(Query.c_str());
  SqlDb.close();
}


///
/// \brief db::init_map_config
/// \param fname
/// \details Установка значений по-умолчанию параметров пользователя
///
void db::init_map_config(const std::string &FilePName)
{
#ifndef NDEBUG
  info("Init new database file.\n");
#endif

  std::string Q = "CREATE TABLE IF NOT EXISTS init ( "
                  "rowid INTEGER PRIMARY KEY, "
                  "key INTEGER, val TEXT );";
  char q[255];
  const char tpl[] = "INSERT INTO init (key, val) VALUES (%d, '%s');";
  sprintf(q, tpl, VIEW_FROM_X,        "0.5");                Q += q;
  sprintf(q, tpl, VIEW_FROM_Y,        "2.0");                Q += q;
  sprintf(q, tpl, VIEW_FROM_Z,        "0.5");                Q += q;
  sprintf(q, tpl, LOOK_AZIM,          "0.0");                Q += q;
  sprintf(q, tpl, LOOK_TANG,          "0.0");                Q += q;

  if(!SqlDb.open(FilePName)) ERR("Can't create dbfile: " + FilePName);
  SqlDb.exec(Q.c_str());
#ifndef NDEBUG
  for(auto &msg: SqlDb.ErrorsList) info(msg);
#endif
  SqlDb.close();
}


///
/// \brief db::init_app_config
/// \param fname
/// \details Установка значений по-умолчанию параметров приложения
///
void db::init_app_config(const std::string &FilePName)
{
#ifndef NDEBUG
  info("Init new database file.\n");
#endif

  std::string Q = "CREATE TABLE IF NOT EXISTS init ( "
                  "rowid INTEGER PRIMARY KEY, "
                  "key INTEGER, val TEXT );";
  char q[255];
  const char tpl[] = "INSERT INTO init (key, val) VALUES (%d, '%s');";
  sprintf(q, tpl, PNG_TEXTURE0,       "tex0_512.png");       Q += q;
  sprintf(q, tpl, DB_TPL_FNAME,       "surf_tpl.db");        Q += q;
  sprintf(q, tpl, SHADER_VERT_SCENE,  "vert.glsl");          Q += q;
  sprintf(q, tpl, SHADER_GEOM_SCENE,  "geom.glsl");          Q += q;
  sprintf(q, tpl, SHADER_FRAG_SCENE,  "frag.glsl");          Q += q;
  sprintf(q, tpl, SHADER_VERT_SCREEN, "scr_vert.glsl");      Q += q;
  sprintf(q, tpl, SHADER_FRAG_SCREEN, "scr_frag.glsl");      Q += q;
  sprintf(q, tpl, WINDOW_SCREEN_FULL, "0");                  Q += q;
  sprintf(q, tpl, WINDOW_WIDTH,       "800");                Q += q;
  sprintf(q, tpl, WINDOW_HEIGHT,      "600");                Q += q;
  sprintf(q, tpl, WINDOW_TOP,         "50");                 Q += q;
  sprintf(q, tpl, WINDOW_LEFT,        "100");                Q += q;

  if(!SqlDb.open(FilePName)) ERR("Can't create dbfile: " + FilePName);
  SqlDb.exec(Q.c_str());
#ifndef NDEBUG
  for(auto &msg: SqlDb.ErrorsList) info(msg);
#endif
  SqlDb.close();
}

} //tr
