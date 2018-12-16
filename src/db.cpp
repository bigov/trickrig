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

wsql db::SqlDb {};
std::string db::MapDir {};    // директория текущей карты
std::string db::MapPFName {}; // имя файла карты
std::string db::CfgMapPFName {}; // файл конфигурации карты/вида
std::string db::CfgAppPFName {}; // файл глобальных настроек приложения


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
  int key;
  std::string val;

  for(auto &row: SqlDb.Table_rows)
  {
    for(auto &p: row)
    {
      if(0 == p.first.find("key")) key = std::stoi(p.second.data());
      if(0 == p.first.find("val")) val = std::string(p.second.data());
    }
    ConfigParams[key] = val;
  }
  SqlDb.close();
  return ConfigParams;
}

///
/// \brief db::open
/// \param PathName - путь к директории данных карты пользователя
///
v_str db::open_map(const std::string &DirPathName)
{
  MapDir = DirPathName;
  CfgMapPFName = MapDir + fname_cfg;
  MapPFName = MapDir + fname_map;
  if(!fs::exists(CfgMapPFName)) init_map_config(CfgMapPFName);
  return load_config(MAP_INIT_SIZE, CfgMapPFName);
}


///
/// \brief db::open
/// \param PathName - путь к директории данных приложения
///
v_str db::open_app(const std::string &DirPathName)
{
  CfgAppPFName = DirPathName + fname_cfg;
  if(!fs::exists(CfgAppPFName)) init_app_config(CfgAppPFName);
  return load_config(APP_INIT_SIZE, CfgAppPFName);
}


///
/// \brief db::save_map_name
/// \param MapName
///
void db::save_map_name(const std::string &MapName)
{
  // Записать в конфиг имя карты, введенное пользователем
  std::string Query = "INSERT INTO init (key, val) VALUES ("+
      std::to_string(MAP_NAME) +", \""+ MapName.c_str() +"\");";
  SqlDb.exec(Query.c_str());
}


///
/// \brief db::save
/// \param Eye
///
void db::save(const tr::camera_3d &Eye)
{
  char q [255]; // буфер для форматирования и передачи строки в запрос
  const char *tpl = "UPDATE init SET val='%s' WHERE key=%d;";
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

  SqlDb.open(CfgMapPFName);
  SqlDb.request_put(Query);
  SqlDb.close();
}


///
/// \brief db::save
/// \param AppWin
///
void db::save(const tr::main_window &AppWin)
{
  char q [255]; // буфер для форматирования и передачи строки в запрос
  const char *tpl = "UPDATE init SET val='%s' WHERE key=%d;";
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

  SqlDb.open(CfgAppPFName);
  SqlDb.request_put(Query);
  SqlDb.close();
}


///
/// \brief db::init_map_config
/// \param fname
/// \details Установка значений по-умолчанию параметров пользователя
///
void db::init_map_config(const std::string &ConfigPname)
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

  if(!SqlDb.open(ConfigPname)) ERR("Can't create dbfile: " + ConfigPname);
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
void db::init_app_config(const std::string &ConfigPname)
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

  if(!SqlDb.open(ConfigPname)) ERR("Can't create dbfile: " + ConfigPname);
  SqlDb.exec(Q.c_str());
#ifndef NDEBUG
  for(auto &msg: SqlDb.ErrorsList) info(msg);
#endif
  SqlDb.close();
}

} //tr
