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
  std::unordered_map<int, std::string> cfg::InitParams {};
  std::string cfg::AssetsDir = "../assets/";

  //## Конструктор
  cfg::cfg()
  {
    set_user_conf_dir();
    return;
  }

  //## Деструктор класса
  cfg::~cfg(void)
  {
    return;
  }

  //## Загрузка конфигурации приложения
  void cfg::load(void)
  {
  /* Загрузка конфига производится отдельным вызовом из главного
   * модуля с перехватом и выводом сообщений об ошибках.
   */
    SqlDb.open(UserConfig);
    SqlDb.exec("SELECT * FROM init;");
    if(SqlDb.num_rows < 1)
    {
      SqlDb.close();
      init_config_db(UserConfig);
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

    return;
  }

  //## Поиск и настройка пользовательского каталога
  void cfg::set_user_conf_dir(void)
  {
#ifdef _WIN32_WINNT
    DS = "\\";
    const char *env_p = getenv("USERPROFILE");
    if(nullptr == env_p) ERR("config::set_user_dir: can'd setup users directory");
    UserDir = std::string(env_p) + std::string("\\AppData\\Roaming");
#else
    DS = "/";
    const char *env_p = getenv("HOME");
    if(nullptr == env_p) ERR("config::set_user_dir: can'd setup users directory");
    UserDir = std::string(env_p);
#endif
    UserDir += DS +".config" + DS + "TrickRig" + DS;
    UserConfig = UserDir + UserConfig;
    return;
  }

  //## Сохрание настроек
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

    p = std::to_string(tr::GlWin.left);
    sprintf(q, tpl, p.c_str(), WINDOW_LEFT);
    Query += q;

    p = std::to_string(tr::GlWin.top);
    sprintf(q, tpl, p.c_str(), WINDOW_TOP);
    Query += q;

    p = std::to_string(tr::GlWin.width);
    sprintf(q, tpl, p.c_str(), WINDOW_WIDTH);
    Query += q;

    p = std::to_string(tr::GlWin.height);
    sprintf(q, tpl, p.c_str(), WINDOW_HEIGHT);
    Query += q;

    SqlDb.request_put(Query);

    return;
  }

  //## Передача клиенту значения параметра
  std::string cfg::get(tr::ENUM_INIT D)
  {
    #ifndef NDEBUG
    if(InitParams[D].empty())
    {
      InitParams[D] = std::string("0");
      tr::info("empty call with DB key=" + std::to_string(D) + "\n");
    }
    #endif

    if(D < ASSETS_LIST_END) return AssetsDir + InitParams[D];
    else return InitParams[D];
  }

} //namespace tr
