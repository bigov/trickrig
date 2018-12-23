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

std::string db::MapDir {};       // директория текущей карты (со слэшем в конце)
std::string db::MapPFName {};    // имя файла карты
std::string db::CfgMapPFName {}; // файл конфигурации карты/вида
std::string db::CfgAppPFName {}; // файл глобальных настроек приложения

std::map<i3d, rig> db::TplRigs_1 {};


///
/// \brief db::db
///
db::db(void)
{
  load_template(1);
}


///
/// \brief db::rigs_loader
/// \param Map
/// \param Start
/// \param End
///
/// \details Загружает блок ригов [P;End] в переданный по ссылке массив.
///
void db::rigs_loader(std::map<i3d, rig> &Map, i3d &Start, i3d &End)
{
  std::string StreamReading = "";

  SqlDb.open(MapPFName);
  i3d P {0,0,0};
  for( P.x = Start.x; P.x < End.x; P.x++ )
    for( P.y = Start.y; P.y < End.y; P.y++ )
      for( P.z = Start.z; P.z < End.z; P.z++)
      {
        Map[P] = load_rig(P, StreamReading);
        Map[P].Origin = P; // .Origin в базе данных не хранится
      }
  SqlDb.close();
}


///
/// \brief rdb::load_template
/// \param level
/// \details загрузка шаблонного фрагмента поверхности размером (tpl_side X tpl_side)
/// для указанного уровня LOD.
///
/// TODO: вобще-то, тут должно быть что-то типа генератора пространства
///
void db::load_template(int level)
{
  if (level != 1) ERR ("rdb::load_space_template need to comple the work");
  SqlDb.open("../assets/surf_tpl.db");

  std::string StreamReading = "";

  i3d P{ 0, 0, 0 };
  for( P.x = 0; P.x < tpl_1_side; P.x++ )
    for( P.z = 0; P.z < tpl_1_side; P.z++ )
    {
      auto R = load_rig(P, StreamReading);
      TplRigs_1[  P                                        ] = R;
      TplRigs_1[ {P.x - tpl_1_side, P.y, P.z             } ] = R;
      TplRigs_1[ {P.x,              P.y, P.z - tpl_1_side} ] = R;
      TplRigs_1[ {P.x - tpl_1_side, P.y, P.z - tpl_1_side} ] = R;
    }

  SqlDb.close();
}


///
/// \brief db::load_rig
/// \param P
/// \param file_name
/// \return
/// \details Загрузка данных. Если указано имя файла, то производится его автоматическое
/// открытие перед обращениием, и закрытие - после получения данных. Если имя файла не
/// передано, то открытие/закрытие файла данных должно производится во внешней процедуре.
/// Это используется для ускорения чтения серии записей, чтобы при обращении к каждой
/// отдельной записи не открывать/закрывать каждый раз файл базы данных
///
rig db::load_rig(const i3d &P, const std::string& file_name)
{
  rig Rig {};

  if(!file_name.empty()) SqlDb.open(file_name);

  SqlDb.select_rig(P.x, P.y, P.z);

  if(SqlDb.Rows.empty()) // если в базе данных нет, то вернуть Rig из шаблона
  {
    Rig = TplRigs_1[ { P.x % tpl_1_side,  P.y % tpl_1_side,  P.z % tpl_1_side } ];
    Rig.born = get_msec();
    return Rig;
  }

  auto Row = SqlDb.Rows.front();
  Rig.born = std::any_cast<int>(Row[0]);

  std::vector<unsigned char> BufVector { std::any_cast<std::vector<unsigned char>>(Row[2]) };
  memcpy(Rig.shift, BufVector.data(), SHIFT_DIGITS * sizeof(float));
  SqlDb.select_snip( std::any_cast<int>(Row[1]) );

  for(auto Row: SqlDb.Rows)
  {
    snip Snip {};
    BufVector.clear();
    BufVector = std::any_cast<std::vector<unsigned char>>(Row[0]);
    memcpy(Snip.data, BufVector.data(), tr::bytes_per_snip);
    Rig.Trick.push_front(Snip);
  }

  if(!file_name.empty()) SqlDb.close();
  return Rig;
}


///
/// \brief db::save_rig
/// \param P
/// \param R
/// \details сохранение в базу данных рига
///
/// Вначале записываем в таблицу снипов данные области рига. При этом
/// индекс области, который будет внесен в таблицу ригов, назначаем
/// по номеру записи первого снипа группы области.
///
/// После этого обновляем/вставляем запись в таблицу ригов с указанием
/// индекса созданой группы
///
void db::save_rig(const i3d &P, const rig *R)
{
  int id_area = 0;
  SqlDb.open(MapPFName);

  for(auto & Snip: R->Trick)
  {
    // Запись снипа
    SqlDb.insert_snip(id_area, Snip.data);

    if(0 == id_area)
    {
      id_area = SqlDb.Result.rowid;
      SqlDb.update_snip( id_area, id_area ); //TODO: !!!THE BUG???
      // Обновить номер группы в записи первого снипа
    }
  }
  // Запись рига
  SqlDb.insert_rig( P.x, P.y, P.z, R->born, id_area, R->shift, SHIFT_DIGITS);
  //DB.request_put(query_buf, R->shift, SHIFT_DIGITS);

  SqlDb.close();
}


///
/// \brief db::save_rigs_block
/// \param From
/// \param To
/// \param RDb
/// \details сохранение в базу данных блока ригов
///
/// Вначале записываем в таблицу снипов данные области рига. При этом
/// индекс области, который будет внесен в таблицу ригов, назначаем
/// по номеру записи первого снипа группы области.
///
/// После этого обновляем/вставляем запись в таблицу ригов с указанием
/// индекса созданой группы
///
void db::save_rigs_block(const i3d &From, const i3d &To, rdb &RDB )
{
  int id_area = 0;
  SqlDb.open(MapPFName);

  for(int x = From.x; x < To.x; x++)
    for(int y = From.y; y < To.y; y++)
      for(int z = From.z; z < To.z; z++)
      {
        rig *R = RDB.get(x, y, z);
        if(nullptr != R)
        {
          id_area = 0;
          for(auto & Snip: R->Trick)
          {
            // Запись снипа
            SqlDb.insert_snip(id_area, Snip.data);

            if(0 == id_area)
            {
              id_area = SqlDb.Result.rowid;
              SqlDb.update_snip( id_area, id_area ); //TODO: !!!THE BUG???
              // Обновить номер группы в записи первого снипа
            }
          }
          // Записать риг
          SqlDb.insert_rig( x, y, z, R->born, id_area, R->shift, SHIFT_DIGITS);
        }
      }
  SqlDb.close();
}


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
  int key = 0;
  std::string val {};

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
/// \brief db::get_map_name
/// \param dbFile
/// \return
/// \details Возвращает значение имени карты из конфига
///
v_ch db::map_name_read(const std::string & dbFile)
{
  SqlDb.open(dbFile);
  std::string Query =
      "SELECT val from init WHERE key="+std::to_string(MAP_NAME)+";";
  SqlDb.exec(Query.c_str());
  auto result = SqlDb.Table_rows.front().front().second;
  SqlDb.close();
  return result;
}


///
/// \brief db::open
/// \param PathName - путь к директории данных карты пользователя (cо слэшем в конце)
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
/// \param MapName
///
void db::map_name_save(const std::string &MapName)
{
  // Записать в конфиг имя карты, введенное пользователем
  std::string Query = "INSERT INTO init (key, val) VALUES ("+
      std::to_string(MAP_NAME) +", \""+ MapName.c_str() +"\");";
  SqlDb.open(CfgMapPFName);
  SqlDb.exec(Query.c_str());
  SqlDb.close();
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

  if(SqlDb.open(CfgMapPFName))
  {
    SqlDb.request_put(Query);
    SqlDb.close();
  }
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

  if(!SqlDb.open(CfgAppPFName)) ERR("Fail: not found app config.");
  SqlDb.request_put(Query);
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
