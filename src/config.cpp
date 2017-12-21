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
  // Инициализация статических членов
  GuiParams tr::cfg::gui = {};
  glm::mat4 MatProjection {}; // матрица проекции 3D сцены
  std::unordered_map<int, std::string> cfg::InitApp {};
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
      InitApp[key] = usr;
    }

    set_size(std::stoi(InitApp[WINDOW_WIDTH]),
             std::stoi(InitApp[WINDOW_HEIGHT]));

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
    return;
  }

  //## Установка соотношения сторон окна
  void cfg::set_size(int w, int h)
  {
    gui.w = w;
    gui.h = h;
    if(gui.h > 0)
      gui.aspect = static_cast<float>(gui.w) / static_cast<float>(gui.h);
    else
      gui.aspect = 1.0f;

    // Град   Радиан
    // 45  |  0,7853981633974483
    // 60  |  1,047197551196598
    // 64  |  1,117010721276371
    // 70  |  1,221730476396031
    tr::MatProjection = glm::perspective(1.118f, gui.aspect, 0.01f, 1000.0f);
  
    return;
  }

  // Статические методы
  int cfg::get_w(void) { return gui.w; }
  int cfg::get_h(void) { return gui.h; }

  //## Передача клиенту значения параметра
  std::string cfg::store(tr::ENUM_INIT D)
  {
    if(D < ASSETS_LIST_END) return AssetsDir + InitApp[D];
    else return InitApp[D];
  }

} //namespace tr
