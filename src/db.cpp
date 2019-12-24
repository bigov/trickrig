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


/*
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
std::unique_ptr<vox> db::get_vox(const i3d& P)
{
  char query[255];
  sprintf(query, "SELECT * FROM voxels WHERE x=%d AND y=%d AND z=%d;",
          P.x, P.y, P.z);
  SqlDb.exec(query);

  if(!SqlDb.Table_rows.empty())
  {
    auto it = SqlDb.Table_rows.front().begin(); it++; it++;
    int size = atoi(it->second.data()); //it++;    // размер стороны
    return std::make_unique<vox>(P, size);
  }
  else return nullptr;
}
*/


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
void db::map_close(std::shared_ptr<glm::vec3> ViewFrom, float* look)
{
  SqlDb.close_in_ram(MapPFName); // закрыть файл данных пространства вокселей

  char q [255]; // буфер для форматирования и передачи строки в запрос
  const char tpl[] = "UPDATE init SET val='%s' WHERE key=%d;";
  std::string p = "";
  std::string Query = "";

  p = std::to_string(ViewFrom->x);
  sprintf(q, tpl, p.c_str(), VIEW_FROM_X);
  Query += q;

  p = std::to_string(ViewFrom->y);
  sprintf(q, tpl, p.c_str(), VIEW_FROM_Y);
  Query += q;

  p = std::to_string(ViewFrom->z);
  sprintf(q, tpl, p.c_str(), VIEW_FROM_Z);
  Query += q;

  p = std::to_string(look[0]);
  sprintf(q, tpl, p.c_str(), LOOK_AZIM);
  Query += q;

  p = std::to_string(look[1]);
  sprintf(q, tpl, p.c_str(), LOOK_TANG);
  Query += q;

  if(SqlDb.open(CfgMapPFName))
  {
    SqlDb.write(Query.c_str());
    SqlDb.close();
  }
}

/*
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
*/

/// DEBUG
void out_char_as_hex (const std::vector<char>& V, int n = 8)
{
  // Так как std::hex работает только с целыми числами, то надо
  // скопировать данные из входного массива <char>V в промежуточный <int>Ints

  std::vector<int> Ints(V.size()/sizeof_y + 1); // +1 если поделилось с остатком
  memcpy(Ints.data(), V.data(), V.size());

  std::cout << "size of array: " << V.size() << "\n"
    << std::hex << std::uppercase;

  int max = Ints.size();
  if(max > n) max = n;

  for(int i = 0; i < max; i++) std::cout << std::setw(sizeof_y * 2)
    << std::setfill('0') << Ints[i] << " ";

  std::cout << std::dec << std::nouppercase << std::endl;
}
void out_char_as_hex (const std::vector<u_int8_t>& Vui, int n = 8)
{
  std::vector<char> Vch (Vui.size(), 0);
  memcpy(Vch.data(), Vui.data(), Vui.size());
  out_char_as_hex(Vch, n);
}


///
/// \brief db::vox_data_delete
/// \param rm_y
/// \param VoxData
/// \details Удаляет блок данных вокса, расположнного на координате rm_y
///
void db::_data_erase(int rm_y, std::vector<u_int8_t>& VoxData)
{
  int y = 0;                // Переменная для приема значения координаты Y
  size_t offset = 0;        // Смещение блока данных вокса в наборе из группы воксов
  size_t vox_data_size = 0; // Размер блока данных текущего вокса

  bool check = (offset + sizeof_y + 1) < VoxData.size();
  while (check)
  {
    memcpy(&y, VoxData.data() + offset, sizeof_y);            // Координата Y
    std::bitset<6> m(VoxData[sizeof_y + offset]);             // Маcка видимых сторон
    vox_data_size = m.count()*bytes_per_side + sizeof_y + 1;  // Размер блока
    if(y == rm_y) VoxData.erase(VoxData.begin()+offset, VoxData.begin()+offset+vox_data_size);
    else offset += vox_data_size;                               // Проверить следующий блок
    // Так как массив может изменяться, то проверяем его размер каждый цикл
    check = (offset + sizeof_y + 1) < VoxData.size();
  }
  return;
}


///
/// \brief db::vox_data_delete
/// \param x
/// \param y
/// \param z
///
void db::vox_data_delete(int x, int y, int z)
{
  char query[127] = {'\0'};

  std::vector<u_int8_t> VoxData = load_vox_data(x, z); // Загрузить "колонку" воксов из БД
  if(VoxData.size() > 0) _data_erase(y, VoxData);      // Удалить блок с координатой у

  if(!VoxData.empty())                                 // Если есть другие воксы, то записать их
  {
    std::sprintf(query, "INSERT OR REPLACE INTO area (x, z, b) VALUES (%d, %d, ?);", x, z);
    SqlDb.request_put(query, VoxData.data(), VoxData.size());
  } else                                               // Если на этой линии воксов больше
  {                                                    //  нет, то удалить запись из БД.
    std::sprintf(query, "DELETE FROM area WHERE( x=%d AND z=%d );", x, z);
    SqlDb.exec(query);
  }
}

///
/// \brief db::save_vox
/// \param V
///
///  \details Данные вершин видимых сторон воксов, передаваемые для рендера в VBO, хранятся в базе
/// данных "RAW"-блоками, т.е. в том-же виде, как и записываются в VBO. Для ускорения операций
/// загрузки данных из БД для идентификации записей используется две из трех координат вокса
/// "Origin.x" и "Origin.z", поэтому данные всех воксов, расположенных вертикально на одной линии Y,
/// записываются в одну "blob" ячейку базы данных последовательно блоками в один общий массив.
///
/// Значение третьей координаты (Y) записывается в начале каждого блока. Длину блока и видимые
/// стороны определяются по маске сторон (функция "vox::get_visibility"), которая записывается в начале
/// каждого блока сразу после значения его координаты Y.
///
void db::vox_data_save (vox* pV)
{
  int y = pV->Origin.y;

  // Загрузить "колонку" воксов из БД
  std::vector<u_int8_t> VoxData = load_vox_data(pV->Origin.x, pV->Origin.z);

  // Удалить из набора, если есть, данные вокса с координатой 'y',
  // так как они будут перезаписаны данными полученного вокса
  if(VoxData.size() > 0) _data_erase(y, VoxData);

  size_t offset = VoxData.size();
  VoxData.resize(VoxData.size() + sizeof_y);
  memcpy(VoxData.data() + offset, &y, sizeof_y);            // Записать координату Y
  VoxData.push_back(pV->get_visibility());                    // Записать маску видимости сторон
  offset = VoxData.size();
  GLfloat buffer[digits_per_side];
  for(u_int8_t side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pV->side_fill_data(side_id, buffer)) continue;
    VoxData.resize(VoxData.size() + bytes_per_side);
    memcpy(VoxData.data() + offset, buffer, bytes_per_side);  // Данные вершин видмой стороны
    offset += bytes_per_side;
  }

  char query[127] = {'\0'};
  std::sprintf(query, "INSERT OR REPLACE INTO area (x, z, b) VALUES (%d, %d, ?);", pV->Origin.x, pV->Origin.z);
  SqlDb.request_put(query, VoxData.data(), VoxData.size());   // Записать в БД
}


///
/// \brief db::load_vox_data
/// \param x
/// \param z
/// \return
///
std::vector<u_int8_t> db::load_vox_data(int x, int z)
{
  char query[127] = {0};
  std::sprintf(query, "SELECT * FROM area WHERE (x=%d AND z=%d);", x, z);
  auto Rows = SqlDb.request_get(&query[0]);

  if(!Rows.empty())
  {
    auto result_row = Rows.front();
    result_row.reverse();
    return result_row.front();
  }
  return std::vector<u_int8_t> {}; // если данных нет, то возвращается пустой вектор
}

///
/// \brief db::erase_vox
/// \param V
///
void db::erase_vox(vox* V)
{
  char q [255]; // буфер для форматирования и передачи строки в запрос
  sprintf(q, "DELETE FROM voxels WHERE (x = %d AND y = %d AND z = %d AND size = %d);",
          V->Origin.x, V->Origin.y, V->Origin.z, V->side_len);
  SqlDb.exec(q);
}


///
/// \brief db::save
/// \param AppWin
///
void db::save_window_layout(const layout& L)
{
  char q [255]; // буфер для форматирования и передачи строки в запрос
  const char tpl[] = "UPDATE init SET val='%s' WHERE key=%d;";
  std::string p = "";
  std::string Query = "";

  p = std::to_string(L.left);
  sprintf(q, tpl, p.c_str(), WINDOW_LEFT);
  Query += q;

  p = std::to_string(L.top);
  sprintf(q, tpl, p.c_str(), WINDOW_TOP);
  Query += q;

  p = std::to_string(L.width);
  sprintf(q, tpl, p.c_str(), WINDOW_WIDTH);
  Query += q;

  p = std::to_string(L.height);
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

  sprintf(q, "INSERT INTO init (key, val) VALUES (%d, %d);", APP_VER_MAJOR, VER_MAJOR);
  Q += q;
  sprintf(q, "INSERT INTO init (key, val) VALUES (%d, %d);", APP_VER_MINOR, VER_MINOR);
  Q += q;
  sprintf(q, "INSERT INTO init (key, val) VALUES (%d, %d);", APP_VER_PATCH, VER_PATCH);
  Q += q;
  sprintf(q, "INSERT INTO init (key, val) VALUES (%d, %d);", APP_VER_TWEAK, VER_TWEAK);
  Q += q;

  if(!SqlDb.open(FilePName)) ERR("Can't create dbfile: " + FilePName);
  SqlDb.exec(Q.c_str());
#ifndef NDEBUG
  for(auto &msg: SqlDb.ErrorsList) info(msg);
#endif
  SqlDb.close();
}

} //tr
