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
v_str db::load_config(size_t params_count, const std::string &FilePath)
{
  v_str ConfigParams {};
  ConfigParams.resize(params_count);
  if(!SqlDb.open(FilePath)) ERR("Can't open DB file: " + FilePath);
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
/// \brief db::open_app
/// \param PathName - путь к директории данных приложения (cо слэшем в конце)
///
v_str db::open_app(const std::string &DirPathName)
{
  CfgAppPFName = DirPathName + fname_cfg;
  if(!fs::exists(CfgAppPFName)) init_app_config(CfgAppPFName);
  return load_config(APP_INIT_SIZE, CfgAppPFName);
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
/// \brief db::map_name_save
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
/// \brief db::map_close
/// \param ViewFrom
/// \param look
///
void db::map_close(std::shared_ptr<glm::vec3> ViewFrom, float* look_dir)
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

  p = std::to_string(look_dir[0]);
  sprintf(q, tpl, p.c_str(), LOOK_AZIM);
  Query += q;

  p = std::to_string(look_dir[1]);
  sprintf(q, tpl, p.c_str(), LOOK_TANG);
  Query += q;

  if(SqlDb.open(CfgMapPFName))
  {
    SqlDb.write(Query.c_str());
    SqlDb.close();
  }
}


///
/// \brief db::blob_data_repack
/// \param VoxData
/// \param DataPack
/// \details Преобразование данных из структуры "data_pack" в бинарный (uchar) массив
///
/// Данные вершин видимых сторон воксов, передаваемые для рендера в VBO, хранятся в базе
/// данных блоками в том-же виде, как записываются в VBO. Для ускорения операций загрузки
/// данных для идентификации записей в БД используется две из трех координат вокса
/// "Origin.x" и "Origin.z". Поэтому данные всех воксов, расположенных вертикально на одной
/// линии Y, записываются в одну "blob" ячейку данных в виде последовательного uchar массива.
///
/// Если с ячейке несколько воксов, то их данные записываются подряд один за другим.
///
std::vector<uchar> db::blob_make(const data_pack& DataPack)
{
  std::vector<uchar> BlobData {};
  if(DataPack.Voxes.empty()) return BlobData;
  for(auto& VoxData: DataPack.Voxes) blob_add_vox_data(BlobData, VoxData);
  return BlobData;
}


///
/// \brief db::blob_add_vox_data
/// \param BlobData
/// \param VoxData
/// \details
/// В начале каждого блока записывается значение координаты Y. Длину блока и
/// видимые стороны определяет битовая маска сторон, которая записывается
/// после значения координаты Y.
///
/// Таким образом структура вокса в базе данных:
///
///  1. координата Y вокса,               : sizeof_y = size_of(int)
///  2. битовая маска видимых сторон      : size_of(uchar)
///  3. данные вершин всех видимых сторон : bytes_per_side * число_видимых_сторон
///
void db::blob_add_vox_data(std::vector<uchar>& BlobData, const vox_data& VoxData)
{
  size_t offset = BlobData.size();                          // Указатель смещения для записи в блоб
  BlobData.resize(offset + sizeof_y);
  memcpy(BlobData.data() + offset, &(VoxData.y), sizeof_y); // Записать координату Y текущего вокса
  offset += sizeof_y;

  std::bitset<SIDES_COUNT> m {0};
  for(auto& S: VoxData.Sides) m.set(S.id);                  // Настроить битовую маску видимых
  BlobData.push_back(static_cast<uchar>(m.to_ulong()));     // сторон и записать ее в блоб
  offset += 1;

  for(auto& SideData: VoxData.Sides)                        // Скопировать данные вершин всех видимых сторон
  {
    BlobData.resize(BlobData.size() + bytes_per_side);
    memcpy(BlobData.data() + offset, SideData.vbo_data, bytes_per_side);
    offset += bytes_per_side;
  }
}

///
/// \brief db::parsing_blob_data
/// \param VoxData
/// \param DataPack
/// \details Преобразование данных из бинарного массива в структуру "data_pack"
///
data_pack db::blob_unpack(const std::vector<uchar>& BlobData)
{
  data_pack ResultPack { 0, 0, {} };
  if(BlobData.empty()) return ResultPack;

  size_t offset = 0;

#ifndef NDEBUG
  if(sizeof_y >= BlobData.size()) ERR("Failure: incorrect blob data");
#endif

  size_t offset_max = BlobData.size() - sizeof_y - 1;

  while (offset < offset_max)                          // Если в блоке несколько воксов, то
  {                                                    // последовательно разбираем каждый из них
    vox_data TheVox { 0, {} };                         // Структура для работы с данными вокса
    memcpy( &(TheVox.y), BlobData.data() + offset, sizeof_y ); // Y-координата вокса
    offset += sizeof_y;
    std::bitset<SIDES_COUNT> m( BlobData[offset] );    // Маcка видимых сторон вокса
    offset += 1;

    int sides_count = m.count();            // число видимых сторон
    if(sides_count == 0) std::cerr << "Err: empty vox data!" << std::endl;
    TheVox.Sides.resize(sides_count);
    char n = 0;                            // индекс данных вектора сторон
    for(char i = 0; i < SIDES_COUNT; ++i)    // Проход по битовой маске сторон:
    {
      if(m.test(i)) TheVox.Sides[n].id = i;  // если сторона присутствует (видимая) - записать ее id
      memcpy(TheVox.Sides[n].vbo_data,       // и скопировать данные вершин, образующих сторону
             BlobData.data() + offset + n * bytes_per_side, bytes_per_side);
      n += 1;
    }
    ResultPack.Voxes.push_back(TheVox);      // Добавить полученные данные в пакет
    offset += m.count() * bytes_per_side;    // Переключить указатель на начало следующего вокса
  }
  return ResultPack;
}


///
/// \brief db::data_pack_vox_remove
/// \param DataPack
/// \param y
/// \return bool
/// \details Удаляет, если есть, данные вокса с координатой 'y'
///
bool db::data_pack_vox_remove( data_pack& DataPack, int y )
{
  if(DataPack.Voxes.empty()) return false;
  auto it = std::find_if(DataPack.Voxes.begin(), DataPack.Voxes.end(),
                         [&y](const auto& Vox){ return Vox.y == y;});
  if(it == DataPack.Voxes.end()) return false;
  DataPack.Voxes.erase(it);
  return true;
}


///
/// \brief db::update_row
/// \param BlobData
/// \param x
/// \param z
///
void db::update_row(const std::vector<uchar>& BlobData, int x, int z)
{
  char query[127] = {'\0'};
  if(!BlobData.empty())                                // Если в блоке есть воксы, то
  {                                                    // обновить запись блока данных
    std::sprintf(query, "INSERT OR REPLACE INTO area (x, z, b) VALUES (%d, %d, ?);", x, z);
    SqlDb.request_put(query, BlobData.data(), BlobData.size());
  }
  else                                                 // Если данных воксов нет,
  {                                                    // то удалить запись из БД
    std::sprintf(query, "DELETE FROM area WHERE( x=%d AND z=%d );", x, z);
    SqlDb.exec(query);
  }
}

///
/// \brief db::vox_data_delete
/// \param x
/// \param y
/// \param z
///
void db::vox_delete(int x, int y, int z)
{
  auto DataPack = load_data_pack(x, z);
  if(!data_pack_vox_remove(DataPack, y))
    std::cerr << "ERROR: Try to delete empty space in the db::vox_data_delete(x,y,z)";
  update_row(blob_make(DataPack), x, z);
}


///
/// \brief db::vox_data_make
/// \param pVox
/// \return
/// \details Создать структуру для манипуляций с данными вокса
///
vox_data db::vox_data_make(vox* pVox)
{
  if(pVox == nullptr) ERR("null ptr on db::vox_data_make(vox* pVox)");
  vox_data VoxData {0, {}};
  VoxData.y = pVox->Origin.y;

  GLfloat buffer[digits_per_side];
  for(uchar side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(pVox->side_fill_data(side_id, buffer))
    {
      side_data Side {};
      Side.id = side_id;
      memcpy(Side.vbo_data, buffer, bytes_per_side);
      VoxData.Sides.push_back(Side);
    }
  }
  return VoxData;
}


///
/// \brief db::vox_data_append
/// \param pVox
///
void db::vox_insert(vox* pVox)
{
  auto DataPack = load_data_pack(pVox->Origin.x, pVox->Origin.z);
  data_pack_vox_remove(DataPack, pVox->Origin.y);
  DataPack.Voxes.push_back(vox_data_make(pVox));
  update_row(blob_make(DataPack), pVox->Origin.x, pVox->Origin.z);
}


///
/// \brief db::load_vox_data
/// \param x
/// \param z
/// \return
///
std::vector<uchar> db::load_blob_data(int x, int z)
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
  return std::vector<uchar> {}; // если данных нет, то возвращается пустой вектор
}


///
/// \brief db::load_data_pack
/// \param x
/// \param z
/// \return
///
data_pack db::load_data_pack(int x, int z)
{
  data_pack result = blob_unpack(load_blob_data(x, z));
  result.x = x;
  result.z = z;
  return result;
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
  std::clog << "Init new database file.\n";
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
  for(auto &msg: SqlDb.ErrorsList) std::clog << msg;
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
  std::clog << "Init new database file.\n";
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
  for(auto &msg: SqlDb.ErrorsList) std::clog << msg;
#endif
  SqlDb.close();
}

} //tr
