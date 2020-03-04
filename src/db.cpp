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
///
/// \details Добавление данных вокса в бинарный (blob) блок для записи в базу данных.
///
/// В начале блока данных каждого вокса записывается значение координаты Y,
/// после координаты Y записывается битовая маска сторон, по которой можно
/// определить длину блока и видимые стороны вокса, чьи данные находятся в блоке.
///
/// Таким образом структура вокса в базе данных:
///
///  1. координата Y вокса,               : sizeof_y = sizeof(int)
///  2. битовая маска видимых сторон      : sizeof(uchar)
///  3. данные вершин всех видимых сторон : bytes_per_side * число_видимых_сторон
///
void db::blob_add_vox_data(std::vector<uchar>& BlobData, const vox_data& VoxData)
{
  size_t offset = BlobData.size();                     // Указатель смещения для записи в блоб
  BlobData.resize(offset + sizeof_y + sizeof(uchar) + bytes_per_face * VoxData.Faces.size());

  memcpy(BlobData.data() + offset, &(VoxData.y), sizeof_y); // Записать координату Y текущего вокса
  offset += sizeof_y;

  std::bitset<SIDES_COUNT> m { 0 };    // Битовая маска видимых сторон
  auto bitset_offset = offset;         // Позиция хранения данных битовой маски вокса

  offset += 1;
  for(auto& Face: VoxData.Faces)  // Скопировать данные вершин сторон вокса
  {
    m.set(Face[0]);               // Настроить битовую маску
    memcpy(BlobData.data() + offset, Face.data() + 1, bytes_per_face);
    offset += bytes_per_face;
  }                                    // записать битовую маску
  BlobData[bitset_offset] = static_cast<uchar>(m.to_ulong());
}

///
/// \brief db::parsing_blob_data
/// \param VoxData
/// \param DataPack
/// \details Преобразование данных из бинарного массива, полученного
/// из базы данных, в структуру "data_pack"
///
data_pack db::blob_unpack(const std::vector<uchar>& BlobData)
{
  data_pack ResultPack {};
  if(BlobData.empty()) return ResultPack;

  size_t offset = 0;

#ifndef NDEBUG
  if(sizeof_y >= BlobData.size()) ERR("Failure: incorrect blob data");
#endif

  size_t offset_max = BlobData.size() - sizeof_y - 1;

  while (offset < offset_max)                          // Если в блоке несколько воксов, то
  {                                                    // последовательно разбираем каждый из них
    vox_data TheVox {};                                // Структура для работы с данными вокса
    memcpy( &(TheVox.y), BlobData.data() + offset, sizeof_y ); // Y-координата вокса
    offset += sizeof_y;
    std::bitset<SIDES_COUNT> bits( BlobData[offset] );    // Маcка видимых сторон вокса
    offset += 1;

#ifndef NDEBUG
    if(bits.count() == 0)
    {
      std::cerr << __PRETTY_FUNCTION__
                << " ERROR: empty vox data on Y=" << TheVox.y << std::endl;
    }
#endif

    for(int i = 0; i < SIDES_COUNT; ++i) // Парсинг по битовой маске
    {
      if(bits.test(i)) // если сторона "i" видимая
      {
        face_t Side {'\0'};
        Side[0] = i;
        memcpy(Side.data() + 1, BlobData.data() + offset, bytes_per_face);
        offset += bytes_per_face;
        TheVox.Faces.push_back(Side);
      }
    }
    ResultPack.Voxes.push_back(TheVox);   // Добавить полученные данные в пакет
  }
  return ResultPack;
}


///
/// \brief db::vox_data_face_on
/// \param VoxData
/// \param face_id
/// \details Заполнение массива данных для указанной грани вокса
///
void db::vox_data_face_on(vox_data& VoxData, unsigned char face_id, const i3d& P, int len)
{
  // TODO: упрощенная реализация - ДОРАБОТАТЬ
  vox V {P, len};
  V.visible_on(face_id);
  GLfloat buff[digits_per_face] = {0.f};
  V.face_fill_data(face_id, buff);

#ifndef NDEBUG
  auto it = std::find_if(VoxData.Faces.begin(), VoxData.Faces.end(),
                         [&face_id](const auto& Face){ return Face[0] == face_id; });
  if(it != VoxData.Faces.end())  // Проверить, нет ли в блоке данных такой стороны
  {                              // Если есть, то удалить,
    VoxData.Faces.erase(it);     // и занести событие в журнал ошибок

    std::cerr << __PRETTY_FUNCTION__
              << " ERROR: add face that exist" << std::endl;
  }
#endif

  face_t Face {};
  Face[0] = face_id;
  memcpy(Face.data()+1, buff, bytes_per_face);

  VoxData.Faces.push_back(Face);
}


///
/// \brief db::osculant_faces_show
/// \param x
/// \param y
/// \param z
/// \param FacesId
/// \details Делает видимыми грани воксов, примыкающих к FacesId граням вокса,
/// удаляемого из точки с коодинатами (x, y, z)
///
///
void db::osculant_faces_show(const int x_base, const int y_base, const int z_base,
                             const std::vector<unsigned char>& FacesId, const int side_len)
{
  for(auto face_id: FacesId)
  {
    i3d P = i3d_near({x_base, y_base, z_base}, face_id, side_len); // Координаты прилегающего вокса, сторону которого надо отобразить

    auto DataPack = blob_unpack(load_blob_data(P.x, P.z)); // Загрузить из БД блок c искомым воксом
    DataPack.x = P.x;
    DataPack.z = P.z;
    DataPack.len = side_len;

#ifndef NDEBUG
    if(DataPack.Voxes.empty()) std::clog
        << __PRETTY_FUNCTION__
        << " DEBUG: empty data_pack on " << P.x << ", " << P.z << std::endl;
#endif

    vox_data VoxData {P.y, {}};

    // Найти в загруженном блоке нужный вокс
    auto it = std::find_if(DataPack.Voxes.begin(), DataPack.Voxes.end(),
                           [&P](const auto& Vox){ return Vox.y == P.y;});
    if(it != DataPack.Voxes.end())
    {
      VoxData = *it;            // Если в БД тут есть вокс, то копируем его,
      DataPack.Voxes.erase(it); // а исходный удаляем чтобы перезаписать
    }
#ifndef NDEBUG
    else {
      std::clog << __PRETTY_FUNCTION__
                << " DEBUG: created vox on Y=" << P.y << std::endl;
    }
#endif

    vox_data_face_on(VoxData, side_opposite(face_id), P, side_len); // Включить сторону, прилегающую к face_id
    DataPack.Voxes.push_back(VoxData);                              // Подготовить бинарный пакет
    update_row(blob_make(DataPack), P.x, P.z);                      // Обновить запись в БД
  }
}


///
/// \brief db::vox_data_delete
/// \param x
/// \param y
/// \param z
/// \details Удаляет из базы данных вокс по указанным координатам
///
void db::vox_delete(const int x, const int y, const int z, const int len)
{
  auto DataPack = blob_unpack(load_blob_data(x, z));
  DataPack.x = x;
  DataPack.z = z;
  DataPack.len = len;

  if(DataPack.Voxes.empty())
  {
#ifndef NDEBUG
    std::cerr << __PRETTY_FUNCTION__
              << " ERROR: remove vox that not exist ("
              << x << ", " << y << ", " << z << ")" << std::endl;
#endif
    return;
  }

  auto it = std::find_if(DataPack.Voxes.begin(), DataPack.Voxes.end(),
                         [&y](const auto& Vox){ return Vox.y == y;});

  if(it == DataPack.Voxes.end())
  {
#ifndef NDEBUG
    std::cerr << __PRETTY_FUNCTION__
              << " ERROR: can't find vox for remove ("
              << x << ", " << y << ", " << z << ")" << std::endl;
#endif
    return;
  }

  auto Vox = *it;                        // Скопировать данные вокса, который будет удален
  DataPack.Voxes.erase(it);              // Удалить вокс
  update_row(blob_make(DataPack), x, z); // Обновить запись в БД

  // Грань вокса невидима, если к ней примыкает грань соседнего вокса.
  // Поэтому после удаления вокса надо найти соседний и сделать у него
  // видимой примыкающую грань.

  // Полный список граней стандартного вокса
  std::vector<unsigned char> FacesAll { SIDE_XP, SIDE_XN, SIDE_YP, SIDE_YN, SIDE_ZP, SIDE_ZN };

  // Список видимых граней удаленного вокса
  std::vector<unsigned char> FacesVisible {};
  for(auto& Face: Vox.Faces) FacesVisible.push_back(Face[0]);

  // Создать и заполнить список невидимых граней удаляемого вокса
  std::vector<unsigned char> FacesUnvisible {};
  std::set_difference(FacesAll.begin(), FacesAll.end(),
                      FacesVisible.begin(), FacesVisible.end(),
                      std::inserter(FacesUnvisible, FacesUnvisible.begin()));

  // Сделать видимыми грани примыкающих воксов
  if(!FacesUnvisible.empty()) osculant_faces_show(DataPack.x, y, DataPack.z,
                                                  FacesUnvisible, DataPack.len);
}


face_t vox_face_make(int, int, int, uchar, int)
{
  return face_t {SIDES_COUNT};
}

///
/// \brief db::vox_data_make
/// \param pVox
/// \return
/// \details Создать грани вокса в указанной точке пространства
///
void db::vox_append(const int x, const int y, const int z, const int len)
{
  auto DataPack = blob_unpack(load_blob_data(x, z));
  DataPack.x = x;
  DataPack.z = z;
  DataPack.len = len;

  auto it = std::find_if(DataPack.Voxes.begin(), DataPack.Voxes.end(),
                         [&y](const auto& Vox){ return Vox.y == y;});

  if(it != DataPack.Voxes.end())
  {
#ifndef NDEBUG
    std::cerr << __PRETTY_FUNCTION__
              << " ERROR: not empty space at the point ("
              << x << ", " << y << ", " << z << ")" << std::endl;
#endif
    DataPack.Voxes.erase(it);              // Удалить вокс
    update_row(blob_make(DataPack), x, z); // Обновить запись в БД
  }

  // Полный список граней стандартного вокса
  unsigned char FacesAll[6] = { SIDE_XP, SIDE_XN, SIDE_YP, SIDE_YN, SIDE_ZP, SIDE_ZN };

  vox_data VoxData {y, {}};
  for(auto face_id: FacesAll)                         // Надо найти соседние воксы и
  {                                                   // скрыть у них примыкающие грани,
    auto Face = vox_face_make(x, y, z, face_id, len); // или сделать грань видимой
    if(Face[0] < SIDES_COUNT) VoxData.Faces.push_back(Face);
  }

  DataPack.Voxes.push_back(VoxData);
  update_row(blob_make(DataPack), x, z); // Обновить запись в БД
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
    auto sec = std::chrono::duration_cast< std::chrono::seconds >
      (std::chrono::system_clock::now().time_since_epoch());
    auto time_point = static_cast<int>(sec.count());

    std::sprintf(query, "INSERT OR REPLACE INTO area (x, z, dt, b) VALUES (%d, %d, %d, ?);",
                 x, z, time_point);
    SqlDb.request_put(query, BlobData.data(), BlobData.size());
  }
  else                                                 // Если данных воксов нет,
  {                                                    // то удалить запись из БД
    std::sprintf(query, "DELETE FROM area WHERE( x=%d AND z=%d );", x, z);
    SqlDb.exec(query);
  }
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

  // Если в базе есть занные для этой области
  if(!Rows.empty())
  {
    auto result_row = Rows.front();
    result_row.reverse();
    return result_row.front();
  }

  // Если данных нет, то возвращается пустой вектор
  return std::vector<uchar> {};
}


///
/// \brief db::load_data_pack
/// \param x
/// \param z
/// \return data_pack
/// \details
///
///
data_pack db::load_data_pack(int x, int z, int len)
{
  data_pack result = blob_unpack(load_blob_data(x, z));
  result.x = x;
  result.z = z;
  result.len = len;

  // Если в базе данных для этой точки пространства нет записи,
  // то ее необходимо создать, сгенерировав поверхность
  if(result.Voxes.empty())
  {
    vox_data Vox {}; // TODO: Доработать генератор случайными элементами
    vox_data_face_on(Vox, SIDE_YP, i3d{x, 0, z}, len);
    result.Voxes.push_back(Vox);
    update_row(blob_make(result), x, z);  // Записать в БД
  }

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
