//============================================================================
//
// file: config.cpp
//
// Управление настройками приложения
//
//============================================================================
/*

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

 */
#include "config.hpp"

namespace tr
{
  // Инициализация глобальных и статических членов
  glm::mat4 MatProjection {}; // матрица проекции 3D сцены
  glm::mat4 MatMVP {};        // Матрица преобразования
  std::unordered_map<int, std::string> cfg::InitParams {};
  std::string cfg::AssetsDir = "../assets/";
  sqlw cfg::SqlDb {};
  std::string cfg::UserDir  = "";          // папка конфигов пользователя
  std::string cfg::DS       = "";          // символ разделителя папок
  std::string cfg::CfgFname = "config.db"; // конфиг пользователя
  camera_3d Eye {};                        // главная камера 3D вида
  main_window AppWin {};                   // параметры окна приложения

  /// Загрузка конфигурации приложения
  /// производится отдельным вызовом из главного модуля
  ///
  void cfg::load(void)
  {
    check_user_dir();

    // При ошибке открытия конфигурации - иницировать новый файл
    if(!SqlDb.open(CfgFname)) init_config_db(CfgFname);

    SqlDb.exec("SELECT * FROM init;");
    if(SqlDb.num_rows < 1)
    {
      SqlDb.close();
      init_config_db(CfgFname);
      SqlDb.exec("SELECT * FROM init;");
      if(SqlDb.num_rows < 1)
      {
        std::string IsFail = "TrickRig init failure!\n";
        for(auto &err: SqlDb.ErrorsList) IsFail += err + "\n";
        ERR(IsFail);
      }
    }

    int key;
    std::string
        val, usr;

    for(auto &row: SqlDb.Table_rows)
    {
      for(auto &p: row)
      {
        if(0 == p.first.find("key")) key = std::stoi(p.second.data());
        if(0 == p.first.find("val")) val = std::string(p.second.data());
        if(0 == p.first.find("usr")) usr = std::string(p.second.data());
      }
      // если нет текущего значения, то используем значение по-умолчанию
      if(usr.empty()) usr = val;
      InitParams[key] = usr;
    }

    // Загрузка настроек камеры вида
    Eye.ViewFrom.x = std::stof(cfg::get(VIEW_FROM_X));
    Eye.ViewFrom.y = std::stof(cfg::get(VIEW_FROM_Y));
    Eye.ViewFrom.z = std::stof(cfg::get(VIEW_FROM_Z));
    Eye.look_a = std::stof(tr::cfg::get(LOOK_AZIM));
    Eye.look_t = std::stof(tr::cfg::get(LOOK_TANG));

    // Загрузка настроек окна приложения
    AppWin.width = static_cast<u_int>(std::stoi(cfg::get(WINDOW_WIDTH)));
    AppWin.height = static_cast<u_int>(std::stoi(cfg::get(WINDOW_HEIGHT)));
    AppWin.top = static_cast<u_int>(std::stoi(cfg::get(WINDOW_TOP)));
    AppWin.left = static_cast<u_int>(std::stoi(cfg::get(WINDOW_LEFT)));
    AppWin.Cursor.x = static_cast<float>(AppWin.width/2) + 0.5f;
    AppWin.Cursor.y = static_cast<float>(AppWin.height/2) + 0.5f;
    AppWin.aspect = static_cast<float>(AppWin.width)
                     / static_cast<float>(AppWin.height);
  }

  ///
  /// Поиск и настройка пользовательского каталога
  ///
  void cfg::check_user_dir(void)
  {
#ifdef _WIN32_WINNT
    DS = "\\";
    const char *env_p = getenv("USERPROFILE");
    if(nullptr == env_p) ERR("config::set_user_dir: can't setup users directory");
    UserDir = std::string(env_p) + std::string("\\AppData\\Roaming");
#else
    DS = "/";
    const char *env_p = getenv("HOME");
    if(nullptr == env_p) ERR("config::set_user_dir: can't setup users directory");
    UserDir = std::string(env_p);
#endif

    UserDir += DS +".config";
    if(!std::filesystem::exists(UserDir))
      std::filesystem::create_directory(UserDir);

    UserDir += DS + "TrickRig";
    if(!std::filesystem::exists(UserDir))
      std::filesystem::create_directory(UserDir);

    if(!std::filesystem::exists(UserDir)) ERR("Can't create: " + UserDir);

    CfgFname = UserDir + DS + CfgFname;
  }


  ///
  /// \brief cfg::create_map
  /// \param map_name
  /// \details Создание в пользовательском каталоге новой карты
  ///
  void cfg::create_map(const std::string &N)
  {
    auto t = std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
    std::string Ts = std::to_string(t);        // число секунд от начала эпохи

    auto DirPName = UserDir + DS + Ts;         // название папки
    if(!std::filesystem::exists(DirPName))
      std::filesystem::create_directory(DirPName);
    else ERR("Err: double map names " + DirPName);

    std::string MapSrc {cfg::get(DB_TPL_FNAME)};  // шаблон карты
    auto MapPName = DirPName + DS + "map.db";     // путь к новой карте

    std::ifstream src(MapSrc, std::ios::binary);  // TODO: контроль чтения
    std::ofstream dst(MapPName, std::ios::binary);
    dst << src.rdbuf();                           // скопировать шаблон карты

    auto CfgPName = DirPName + DS + "config.db";  // путь к файлу конфига
    init_config_db(CfgPName);                     // создать новый конфиг

    sqlw Db{CfgPName};
    Db.open();

#ifndef NDEBUG
  for(auto &msg: Db.ErrorsList) tr::info(msg);
#endif

    // Записать в конфиг имя карты, введенное пользователем
    std::string Query = "INSERT INTO init (key, val) VALUES ("+
        std::to_string(MAP_NAME) +", \""+ N.c_str() +"\");";
    Db.exec(Query.c_str());

#ifndef NDEBUG
  for(auto &msg: Db.ErrorsList) tr::info(msg);
#endif

    Db.close();
  }


  ///
  /// Сохрание настроек текущей сессии при закрытии программы
  ///
  void cfg::save(void)
  {
    char q [255]; // буфер для форматирования и передачи строки в запрос
    const char *tpl = "UPDATE init SET val='%s' WHERE key=%d;";
    std::string p = "";
    std::string Query = "";

    p = std::to_string(tr::Eye.ViewFrom.x);
    sprintf(q, tpl, p.c_str(), VIEW_FROM_X);
    Query += q;

    p = std::to_string(tr::Eye.ViewFrom.y);
    sprintf(q, tpl, p.c_str(), VIEW_FROM_Y);
    Query += q;

    p = std::to_string(tr::Eye.ViewFrom.z);
    sprintf(q, tpl, p.c_str(), VIEW_FROM_Z);
    Query += q;

    p = std::to_string(tr::Eye.look_a);
    sprintf(q, tpl, p.c_str(), LOOK_AZIM);
    Query += q;

    p = std::to_string(tr::Eye.look_t);
    sprintf(q, tpl, p.c_str(), LOOK_TANG);
    Query += q;

    p = std::to_string(tr::AppWin.left);
    sprintf(q, tpl, p.c_str(), WINDOW_LEFT);
    Query += q;

    p = std::to_string(tr::AppWin.top);
    sprintf(q, tpl, p.c_str(), WINDOW_TOP);
    Query += q;

    p = std::to_string(tr::AppWin.width);
    sprintf(q, tpl, p.c_str(), WINDOW_WIDTH);
    Query += q;

    p = std::to_string(tr::AppWin.height);
    sprintf(q, tpl, p.c_str(), WINDOW_HEIGHT);
    Query += q;

    SqlDb.request_put(Query);
  }

  ///
  /// Передача клиенту значения параметра
  ///
  std::string cfg::get(tr::ENUM_INIT D)
  {
    #ifndef NDEBUG
    if(InitParams[D].empty())
    {
      InitParams[D] = std::string("0");
      tr::info("empty call with DB key=" + std::to_string(D) + "\n");
    }
    #endif

    // Имя файла в папке "assets"
    if(D < ASSETS_LIST_END) return AssetsDir + InitParams[D];
    else return InitParams[D];
  }

} //namespace tr
