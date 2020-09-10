/*
 * file: config.cpp
 *
 * Управление настройками приложения
 *
 *
 */

#include "config.hpp"

namespace tr{

#ifdef _WIN32_WINNT
extern "C" {
#include <sec_api/stdlib_s.h>
errno_t getenv_s(
  size_t     *ret_required_buf_size,
  char       *buf,
  size_t      buf_size_in_bytes,
  const char *name );
}
#endif


// Инициализация статических членов
db          cfg::DataBase  {};
v_str       cfg::AppParams {}; // параметры конфигурации приложения
v_str       cfg::MapParams {}; // параметры когфигурации карты
std::string cfg::AssetsDir {}; // папка служебных файлов
std::string cfg::UserDirDB   {}; // папка конфигов пользователя
std::string cfg::DS        {}; // символ разделителя папок
std::string cfg::CfgFname  {}; // конфиг, выбранный пользователем
layout      cfg::WinLayout {}; // размер и положение главного окна

///
/// \brief cfg::user_dir
/// \return
///
/// \details Возвращает полный путь к папке конфигов пользователя
///
std::string cfg::app_data_dir(void)
{
  return UserDirDB;
}


///
/// Загрузка параметров сессии карты
///
void cfg::map_view_load(const std::string &DirName, std::shared_ptr<glm::vec3> ViewFrom, float* look)
{
  MapParams = DataBase.map_open(DirName + DS);

  // Загрузка настроек камеры вида
  ViewFrom->x = std::stof(MapParams[VIEW_FROM_X]);
  ViewFrom->y = std::stof(MapParams[VIEW_FROM_Y]);
  ViewFrom->z = std::stof(MapParams[VIEW_FROM_Z]);
  look[0] = std::stof(MapParams[LOOK_AZIM]);

  // Если взгляд направлен выше линии горизонта (look[1] > 0), то при загрузке карты
  // сориентироваться в пространстве не сразу удобно. Комфортнее, если взгляд будет
  // направлен немного вниз, "под ноги":

  // look[1] = std::stof(MapParams[LOOK_TANG]);
  // if(look[1] > 0) look[1] = -0.2f;
  look[1] = -0.25f;
}


///
/// \brief cfg::map_name
/// \param FolderName
/// \return
/// \details Читает имя карты из указанной папки
std::string cfg::map_name(const std::string &FolderName)
{
  auto config = FolderName + DS + fname_cfg;
  v_ch Name = DataBase.map_name_read(config);
  if(Name.size() < 1) return "no name";
  else return std::string(Name.data());
}


///
/// Загрузка параметров приложения
///
void cfg::load(char** argv)
{
#ifdef _WIN32_WINNT
  DS = "\\";
#else
  DS = "/";
#endif

  fs::path p = argv[0];
  //AssetsDir = fs::absolute(p).parent_path().string() + DS + "assets";
  AssetsDir = "assets";

  if(!fs::exists(AssetsDir)) ERR("\nNot found assets dir: " + AssetsDir);

  set_user_dir();
  AppParams = DataBase.open_app(UserDirDB + DS);

  if(std::stoi(AppParams[APP_VER_MAJOR]) != VER_MAJOR) ERR("Incompatible APP_MAJOR database version");
  if(std::stoi(AppParams[APP_VER_MINOR]) != VER_MINOR) ERR("Incompatible APP_MINOR database version");

  WinLayout.width = static_cast<uint>(std::stoi(AppParams[WINDOW_WIDTH]));
  WinLayout.height = static_cast<uint>(std::stoi(AppParams[WINDOW_HEIGHT]));
  WinLayout.left = static_cast<uint>(std::stoi(AppParams[WINDOW_LEFT]));
  WinLayout.top = static_cast<uint>(std::stoi(AppParams[WINDOW_TOP]));

}


///
/// Поиск и настройка пользовательского каталога
///
void cfg::set_user_dir(void)
{
#ifdef _WIN32_WINNT
  char env_key[] = "USERPROFILE";
  size_t requiredSize;
  getenv_s( &requiredSize, nullptr, 0, env_key);
  std::vector<char> libvar {};
  libvar.resize(requiredSize);
  getenv_s( &requiredSize, libvar.data(), requiredSize, env_key );
  UserDirDB = std::string(libvar.data()) + std::string("\\AppData\\Roaming");
#else
  DS = "/";
  const char *env_p = getenv("HOME");
  if(nullptr == env_p) ERR("config::set_user_dir: can't setup users directory");
  UserDir = std::string(env_p);
  UserDir += DS +".config";
#endif

#ifndef NDEBUG
  // На время разработки конфиг пользователя и база данных данных расположена в папке приложения
  UserDirDB = AssetsDir + DS + "database";
#else
  if(!fs::exists(UserDir)) fs::create_directory(UserDir);
  UserDir += DS + "TrickRig";
#endif

  if(!fs::exists(UserDirDB)) fs::create_directory(UserDirDB);
  if(!fs::exists(UserDirDB)) ERR("Fatal error: can't create: " + UserDirDB);
}


///
/// \brief cfg::create_map
/// \param map_name
/// \details Создание в пользовательском каталоге новой карты
///
std::string cfg::create_map(const std::string &MapName)
{
  auto MapSrc { UserDirDB + DS + "space.db" };    // шаблон карты
  if(!fs::exists(MapSrc)) ERR("\nFAILURE: not found template: " + MapSrc);

  // число секунд от начала эпохи - ID новой карты
  auto t_id = std::chrono::duration_cast<std::chrono::seconds>
      (std::chrono::system_clock::now().time_since_epoch()).count();

  auto DirPName { UserDirDB + DS + std::to_string(t_id) }; // название папки
  if(!fs::exists(DirPName)) fs::create_directory(DirPName);
  else ERR("Err: map dir exist: " + DirPName);

  auto MapPathName { DirPName + DS + fname_map};  // новая карта

  std::ifstream src(MapSrc, std::ios::binary);    // TODO: контроль чтения
  std::ofstream dst(MapPathName, std::ios::binary);
  dst << src.rdbuf();                             // скопировать шаблон карты

  DataBase.init_map_config(DirPName + DS + fname_cfg);
  DataBase.map_name_save(DirPName + DS, MapName); // записать название пользователя
  return DirPName;
}


///
/// Сохрание настроек текущей сессии при закрытии программы
///
void cfg::save(const layout& WindowLayout)
{
  DataBase.save_window_layout(WindowLayout);
}


///
/// Сохранить настройки положения камеры и закрыть карту
///
void cfg::map_view_save(std::shared_ptr<glm::vec3> View, float* look_dir)
{
  DataBase.map_close(View, look_dir);
}


///
/// Передача клиенту значения параметра
///
std::string cfg::map_key(MAP_INIT D)
{
#ifndef NDEBUG
  if(MapParams[D].empty())
  {
    MapParams[D] = std::string("0");
    std::cerr << "empty call with DB key=" << std::to_string(D) << std::endl;
  }
#endif
  return MapParams[D];
}


///
/// Передача клиенту значения параметра
///
std::string cfg::app_key(APP_INIT D)
{
  // Имя файла шейдера
  if((D >= SHADER_VERT_SCENE) and (D <= SHADER_FRAG_SCREEN))
    return AssetsDir + DS + "shaders" + DS + AppParams[D];

  // Имя файла в папке "assets"
  if(D < ASSETS_LIST_END) return AssetsDir + DS + AppParams[D];
  else return AppParams[D];
}


///
/// \brief file_path_texture
/// \return
///
std::string cfg::file_path_texture(APP_INIT n)
{
#ifndef NDEBUG
  assert((n < TEXTURES_COUNT) && "Недопустимый индекс текстуры.");
#endif

  std::string FileName = AssetsDir + DS + "textures" + DS + AppParams[n];

#ifndef NDEBUG
  assert (fs::exists(FileName) && "Отсутствует файл текстуры.");
#endif

  return FileName;
}

} //namespace tr
