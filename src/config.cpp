//============================================================================
//
// file: config.cpp
//
// Управление настройками приложения
//
//============================================================================
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

    int key; std::string val, usr;
    for(auto &row: SqlDb.rows)
    {
      for(auto &p: row)
      {
        if(0 == p.first.find("key")) key = std::stoi(p.second);
        if(0 == p.first.find("val")) val = p.second;
        if(0 == p.first.find("usr")) usr = p.second;
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
    char query [255]; // буфер для форматирования и передачи строки в запрос
    std::string p = std::to_string(tr::ViewFrom.y);

    sprintf(query, "UPDATE init SET val='%s' WHERE key=%d;",
            p.c_str(), VIEW_FROM_Y);
    SqlDb.exec(query);
    for(auto &msg: SqlDb.ErrorsList) tr::info(msg);

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
