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
/// В начале блока данных каждого вокса записывается значение координаты Y,
/// после координаты Y записывается битовая маска сторон, по которой можно
/// определить длину блока и id (видимых) сторон вокса, хранящихся в блобе.
///
/// Таким образом структура вокса в базе данных:
///
///  1. координата Y вокса,               : sizeof_y = sizeof(int)
///  2. битовая маска видимых сторон      : sizeof(uchar)
///  3. данные вершин всех видимых сторон : bytes_per_side * число_видимых_сторон
///
std::vector<uchar> db::blob_make(data_pack& DataPack)
{
  if(DataPack.Voxes.empty()) return std::vector<uchar> {};

  for(auto& Vox: DataPack.Voxes) // Перед упаковкой воксов в пакет, следует в каждом из
  {                              // них упорядочить очередность следования видимых сторон
    std::sort(Vox.Faces.begin(), Vox.Faces.end(),
              [](const face_t& f1, const face_t& f2 ){ return f1[0] < f2[0]; });
  }

  std::vector<uchar> BlobData {};
  for(auto& VoxData: DataPack.Voxes)
  {
    if(VoxData.Faces.empty()) continue; // если видимых сторон нет, то в базу данные не вносим

    size_t offset = BlobData.size();                     // Указатель смещения для записи в блоб
    BlobData.resize(offset + sizeof_y + sizeof(uchar) + bytes_per_face * VoxData.Faces.size());

    memcpy(BlobData.data() + offset, &(VoxData.y), sizeof_y); // Записать координату Y текущего вокса
    offset += sizeof_y;

    std::bitset<SIDES_COUNT> m { 0 };  // Битовая маска видимых сторон вокса
    auto bitset_offset = offset;       // Запомнить адрес хранения данных битовой маски
    offset += 1;

    for(auto& Face: VoxData.Faces)     // Скопировать данные вершин сторон вокса
    {
      m.set(Face[0]);                  // Настроить битовую маску
      memcpy(BlobData.data() + offset, Face.data() + 1, bytes_per_face);
      offset += bytes_per_face;
    }                                  // записать битовую маску
    BlobData[bitset_offset] = static_cast<uchar>(m.to_ulong());
  }

  return BlobData;
}


///
/// \brief db::load_data
/// \param x
/// \param z
/// \return data_pack
/// \details Загрузка из базы данных пакета в формате "data_pack"
///
data_pack db::load_data(const int x, const int z)
{
  data_pack ResultPack {};
  ResultPack.x = x;
  ResultPack.z = z;

  char query[127] = {0};
  std::sprintf(query, "SELECT * FROM area WHERE (x=%d AND z=%d);", x, z);
  auto BlobData = SqlDb.request_get(&query[0]);

  // Если в базе нет данных, то возвращаем пустой результат
  if(BlobData.empty()) return ResultPack;

  assert(sizeof_y < BlobData.size());
  size_t offset = 0;
  size_t offset_max = BlobData.size() - sizeof_y - 1;

  while (offset < offset_max)                           // Если в блоке несколько воксов, то
  {                                                     // последовательно разбираем каждый из них
    vox_data TheVox {};                                 // Структура для работы с данными вокса
    memcpy( &(TheVox.y), BlobData.data() + offset, sizeof_y ); // Y-координата вокса
    offset += sizeof_y;
    std::bitset<SIDES_COUNT> bits( BlobData[offset] );  // Маcка видимых сторон вокса
    offset += 1;

#ifndef NDEBUG
    if(bits.count() == 0)
    {
      std::cerr << std::endl << __PRETTY_FUNCTION__ << std::endl
                << "ERROR: empty vox data on "
                << x << "," << TheVox.y << "," << z << std::endl;
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
/// \brief db::vox_data_face_off
/// \param VoxData
/// \param face_id
/// \details Удаление данных поверхности вокса
///
void db::vox_face_erase(vox_data& VoxData, unsigned char face_id)
{
  // Найти в блоке данных вокса сторону "face_id"
  auto it = std::find_if(VoxData.Faces.begin(), VoxData.Faces.end(),
                         [&face_id](const auto& Face){ return Face[0] == face_id; });
  // Если есть, то удалить
  if(it != VoxData.Faces.end())
  {
    VoxData.Faces.erase(it);
  }
#ifndef NDEBUG
  else
  {
    std::cerr << std::endl << __PRETTY_FUNCTION__ << std::endl
              << "Face id="<< face_id <<" not exists." << std::endl;
  }
#endif
}


///
/// \brief db::vox_by_face
/// \param DataPack
/// \param y
/// \param face_id
/// \return
/// \details Поиск вокса с координатой Y
///
vox_t::iterator db::vox_find(vox_t& Voxes, const int y)
{
  auto VoxIt = std::find_if(Voxes.begin(), Voxes.end(),
                         [&y](const auto& Vox){ return Vox.y == y;});
  return VoxIt;
}


///
/// \brief db::vox_data_delete
/// \param x
/// \param y
/// \param z
/// \details Удаляет из базы данных вокс по указанным координатам
///
/// У вокса любая грань невидима в том случае, если к ней вплотную примыкает соседний вокс.
/// Поэтому, при удалении вокса, грани примыкающих воксов надо сделать видимыми. Если
/// со стороны невидимой грани в БД вокса не найден, то он должен быть создан.
///
void db::vox_delete(const int x, const int y, const int z, const int len)
{
  auto DataPack = load_data(x, z);
  if(DataPack.Voxes.empty()) return;
  auto it = vox_find(DataPack.Voxes, y);
  if(it == DataPack.Voxes.end()) return;

  std::vector<unsigned char> VisibleFacesList {};                 // Создать список видимых
  for(auto& Face: it->Faces) VisibleFacesList.push_back(Face[0]); // граней удаляемого вокса.
  DataPack.Voxes.erase(it);                                       // Удалить вокс.

  // Список боковых граней стандарного вокса
  std::vector<unsigned char> FacesAll { SIDE_XP, SIDE_XN, SIDE_YP, SIDE_YN, SIDE_ZP, SIDE_ZN };

  // По списку видимых граней удаляемого вокса построить список невидимых граней
  std::vector<unsigned char> FacesUnvisible {};
  std::set_difference(FacesAll.begin(), FacesAll.end(),
                      VisibleFacesList.begin(), VisibleFacesList.end(),
                      std::inserter(FacesUnvisible, FacesUnvisible.begin()));

  // Построить грани воксов, расположенных вплотную к воксу, который был удален.
  for(auto face_id: FacesUnvisible)
  {
    auto _P = i3d_near({x, y, z}, face_id, len);      // Координаты прилегающего вокса
    auto _face_id = side_opposite(face_id);           // id прилегающей стороны
    bool this_pack = (_P.x == x) && (_P.z == z);
    data_pack _Pack {};

    if(!this_pack) _Pack = load_data(_P.x, _P.z);    // Загрузить блок
    else _Pack = DataPack;

    auto it = vox_find(_Pack.Voxes, _P.y);            // Получить указатель на вокс
    if(it == _Pack.Voxes.end())
    {                                                 // Если в точке _P.y вокса нет,
      _Pack.Voxes.push_back(vox_data{_P.y, {}});      // то его надо создать,
      it = vox_find(_Pack.Voxes, _P.y);               // и повторно настроить указатель
    }
    it->Faces.push_back(face_gen(_P, len, _face_id));        // Создать сторону, прилегающую к face_id
    if(!this_pack) update_row(blob_make(_Pack), _P.x, _P.z); // Обновить запись в БД
    else DataPack = _Pack;                                   // или вернуть изменения в DataPack
  }
  update_row(blob_make(DataPack), x, z);              // Обновить запись о пакете в БД
}


///
/// \brief db::vox_data_make
/// \param pVox
/// \return
/// \details В указанной точке пространства создает вокс указанного размера
///
void db::vox_append(const int x, const int y, const int z, const int len)
{
  auto DataPack = load_data(x, z); // Загрузить из БД пакет
  if(vox_find(DataPack.Voxes, y) != DataPack.Voxes.end())
  {
#ifndef NDEBUG
    std::cerr << std::endl << __PRETTY_FUNCTION__ << std::endl
              << "The vox exist on " << x << "," << y << "," << z << std::endl;
#endif
    return; // Тут вокса не должно быть - поэтому выйти.
  }

  const i3d P = {x, y, z};
  vox_data NewVox {y, {}}; // cоздать новый вокс для добавления в пакет

  data_pack _Pack {};
  for(uchar face_id = 0; face_id < SIDES_COUNT; ++face_id)
  {
    auto _P = i3d_near(P, face_id, len);        // Координата примыкающего вокса
    bool this_pack = (P.x == _P.x) && (P.z ==_P.z);
    auto _face_id = side_opposite(face_id);     // ID примыкающей грани

    if(this_pack) _Pack = DataPack;             // Если это исходный блок, то работаем с ним,
    else _Pack = load_data(_P.x, _P.z);         // иначе - загрузить из БД примыкающий блок

    auto _VoxIt = vox_find(_Pack.Voxes, _P.y);  // Найти указатель на примыкающий вокс
    if( _VoxIt != _Pack.Voxes.end())
    {                                           // Если там вокс, то удалить
      vox_face_erase(*_VoxIt, _face_id);        // у него примыкающую грань,
      if(!this_pack) update_row(blob_make(_Pack), _P.x, _P.z);
      else DataPack = _Pack;                    // Если это исходный блок, то вернуть в него изменения
    }
    else {                                               // или
      NewVox.Faces.push_back(face_gen(P, len, face_id)); // построить с этой стороны новую грань
    }
  }

  DataPack.Voxes.push_back(NewVox);             // Добавить подготовленный вокс в пакет
  update_row(blob_make(DataPack), x, z);        // и обновить запись пакета в БД
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
/// \brief db::load_data_pack
/// \param x
/// \param z
/// \return data_pack
/// \details Загрузка данных в модуль отображения пространства. Особенностью
/// функции является правило - "не должно быть пустых пакетов". Поэтому,
/// если в базе данных для указанной точки пространства поверхность еще не
/// сгенерирована, то функция автоматически генерирует поверхность по-умолчанию,
/// создает запись в базе данных и передает пакет в модуль отображения.
///
data_pack db::area_load(int x, int z, int len)
{
  data_pack DataPack = load_data(x, z);

  // Если в базе данных для этой точки пространства нет записи,
  // то ее необходимо создать, сгенерировав поверхность
  if(DataPack.Voxes.empty())
  {
    vox_data VoxData {}; // TODO: Доработать генератор случайными элементами
    VoxData.Faces.push_back(face_gen(i3d{x, 0, z}, len, SIDE_YP));
    DataPack.Voxes.push_back(VoxData);
    update_row(blob_make(DataPack), x, z);  // Записать в БД
  }

  return DataPack;
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
  sprintf(q, tpl, SHADER_VERT_SCENE,  "3d_vert.glsl");       Q += q;
  sprintf(q, tpl, SHADER_GEOM_SCENE,  "3d_geom.glsl");       Q += q;
  sprintf(q, tpl, SHADER_FRAG_SCENE,  "3d_frag.glsl");       Q += q;
  sprintf(q, tpl, SHADER_VERT_SCREEN, "win_vert.glsl");      Q += q;
  sprintf(q, tpl, SHADER_FRAG_SCREEN, "win_frag.glsl");      Q += q;
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
