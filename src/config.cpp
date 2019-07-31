/*
 * file: config.cpp
 *
 * Управление настройками приложения
 *
 *
 */

#include "config.hpp"
//#include "io.hpp"

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

// Инициализация глобальных объектов
glm::mat4 MatProjection {}; // матрица проекции 3D сцены
float zNear = 1.f;          // расстояние до ближней плоскости матрицы проекции
float zFar  = 10000.f;      // расстояние до дальней плоскости матрицы проекции
glm::mat4 MatMVP        {}; // Матрица преобразования
camera_3d Eye           {}; // главная камера 3D вида
main_window WinParams      {}; // параметры окна приложения

// Инициализация статических членов
db cfg::DataBase {};

v_str cfg::AppParams {}; // параметры конфигурации приложения
v_str cfg::MapParams {}; // параметры когфигурации карты
std::string cfg::AssetsDir {}; // папка служебных файлов
std::string cfg::UserDir   {}; // папка конфигов пользователя
std::string cfg::DS        {}; // символ разделителя папок
std::string cfg::CfgFname  {}; // конфиг, выбранный пользователем

///
/// \brief cfg::user_dir
/// \return
///
/// \details Возвращает полный путь к папке конфигов пользователя
///
std::string cfg::user_dir(void)
{
  return UserDir;
}


///
/// Загрузка параметров сессии карты
///
void cfg::load_map_cfg(const std::string &DirName)
{
  MapParams = DataBase.map_open(DirName + DS);

  // Загрузка настроек камеры вида
  Eye.ViewFrom.x = std::stof(MapParams[VIEW_FROM_X]);
  Eye.ViewFrom.y = std::stof(MapParams[VIEW_FROM_Y]);
  Eye.ViewFrom.z = std::stof(MapParams[VIEW_FROM_Z]);
  Eye.look_a = std::stof(MapParams[LOOK_AZIM]);
  Eye.look_t = std::stof(MapParams[LOOK_TANG]);
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
void cfg::load_app_cfg(void)
{
  set_user_dir();
  AssetsDir = ".." + DS + "assets";
  AppParams = DataBase.open_app(UserDir + DS);

  // Загрузка настроек окна приложения
  WinParams.width = static_cast<u_int>(std::stoi(AppParams[WINDOW_WIDTH]));
  WinParams.height = static_cast<u_int>(std::stoi(AppParams[WINDOW_HEIGHT]));
  WinParams.top = static_cast<u_int>(std::stoi(AppParams[WINDOW_TOP]));
  WinParams.left = static_cast<u_int>(std::stoi(AppParams[WINDOW_LEFT]));
  WinParams.Cursor.x = static_cast<float>(WinParams.width/2) + 0.5f;
  WinParams.Cursor.y = static_cast<float>(WinParams.height/2) + 0.5f;
  WinParams.aspect = static_cast<float>(WinParams.width)
                   / static_cast<float>(WinParams.height);

  MatProjection = glm::perspective(1.118f, WinParams.aspect, zNear, zFar);
}


///
/// Поиск и настройка пользовательского каталога
///
void cfg::set_user_dir(void)
{
#ifdef _WIN32_WINNT
  DS = "\\";

//    const char *env_p = getenv("USERPROFILE");
//    if(nullptr == env_p) ERR("config::set_user_dir: can't setup users directory");
//    UserDir = std::string(env_p) + std::string("\\AppData\\Roaming");

  char env_key[] = "USERPROFILE";
  size_t requiredSize;
  getenv_s( &requiredSize, NULL, 0, env_key);
  std::vector<char> libvar {};
  libvar.resize(requiredSize);
  getenv_s( &requiredSize, libvar.data(), requiredSize, env_key );
  UserDir = std::string(libvar.data()) + std::string("\\AppData\\Roaming");

#else
  DS = "/";
  const char *env_p = getenv("HOME");
  if(nullptr == env_p) ERR("config::set_user_dir: can't setup users directory");
  UserDir = std::string(env_p);
#endif

  UserDir += DS +".config";
   if(!fs::exists(UserDir)) fs::create_directory(UserDir);
  UserDir += DS + "TrickRig";
  if(!fs::exists(UserDir)) fs::create_directory(UserDir);
  if(!fs::exists(UserDir)) ERR("Can't create: " + UserDir);
}


///
/// \brief cfg::create_map
/// \param map_name
/// \details Создание в пользовательском каталоге новой карты
///
std::string cfg::create_map(const std::string &MapName)
{
  // число секунд от начала эпохи
  auto t = std::chrono::duration_cast<std::chrono::seconds>
      (std::chrono::system_clock::now().time_since_epoch()).count();

  auto DirPName { UserDir + DS + std::to_string(t) }; // название папки
  if(!fs::exists(DirPName)) fs::create_directory(DirPName);
  else ERR("Err: map dir exist: " + DirPName);

  auto MapSrc { AssetsDir + DS + "space.db" };   // шаблон карты
  auto MapPathName { DirPName + DS + fname_map};    // новая карта

  std::ifstream src(MapSrc, std::ios::binary);   // TODO: контроль чтения
  std::ofstream dst(MapPathName, std::ios::binary);
  dst << src.rdbuf();                            // скопировать шаблон карты

  DataBase.init_map_config(DirPName + DS + fname_cfg);
  DataBase.map_name_save(DirPName + DS, MapName); // записать название пользователя
  return DirPName;
}


///
/// Сохрание настроек текущей сессии при закрытии программы
///
void cfg::save_app(void)
{
  DataBase.save_window_params(WinParams);
}


///
/// Сохранить настройки положения камеры и закрыть карту
///
void cfg::save_map_view(void)
{
  DataBase.map_close(Eye);
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
    info("empty call with DB key=" + std::to_string(D) + "\n");
  }
#endif
  return MapParams[D];
}


///
/// Передача клиенту значения параметра
///
std::string cfg::app_key(APP_INIT D)
{
#ifndef NDEBUG
  if(AppParams[D].empty())
  {
    AppParams[D] = std::string("0");
    info("empty call with DB key=" + std::to_string(D) + "\n");
  }
#endif

  // Имя файла в папке "assets"
  if(D < ASSETS_LIST_END) return AssetsDir + DS + AppParams[D];
  else return AppParams[D];
}

} //namespace tr
