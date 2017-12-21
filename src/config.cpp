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
  GuiParams tr::config::gui = {};
  glm::mat4 MatProjection {}; // матрица проекции 3D сцены
  std::unordered_map<tr::FileDestination, std::string> config::fp_name {};

  //## Конструктор
  config::config()
  {
    set_user_conf_dir();
    return;
  }

  //## Деструктор класса
  config::~config(void)
  {
    return;
  }

  //## Загрузка конфигурации приложения
  void config::load(void)
  {
  /* Загрузка конфига производится отдельным вызовом из главного
   * модуля с перехватом и выводом сообщений об ошибках.
   */

    SqlDb.set_db_name(UserTrConfDir + "config.db");
    SqlDb.open();
    SqlDb.close();

    value = 0;
    set_size(800, 600);
    std::string dir = "../assets/";

    fp_name[FONT_FNAME] = dir + "DejaVuSansMono.ttf";
    fp_name[HUD_FNAME] = dir + "hud.png";
    fp_name[TEXTURE_FNAME] = dir + "tex0_512.png";

    fp_name[SHADER_VERT_SCENE] = dir + "vert.glsl";
    fp_name[SHADER_GEOM_SCENE] = dir + "geom.glsl";
    fp_name[SHADER_FRAG_SCENE] = dir + "frag.glsl";

    fp_name[SHADER_VERT_SCREEN] = dir + "scr_vert.glsl";
    fp_name[SHADER_FRAG_SCREEN] = dir + "scr_frag.glsl";

    return;
  }

  //## Поиск и настройка пользовательского каталога
  void config::set_user_conf_dir(void)
  {
#ifdef _WIN32_WINNT
    DS = "\\";
    const char *env_p = getenv("USERPROFILE");
    if(nullptr == env_p) ERR("config::set_user_dir: can'd setup users directory");
    UserTrConfDir = std::string(env_p) + std::string("\\AppData\\Roaming");
#else
    DS = "/";
    const char *env_p = getenv("HOME");
    if(nullptr == env_p) ERR("config::set_user_dir: can'd setup users directory");
    UserTrConfDir = std::string(env_p);
#endif
    UserTrConfDir += DS +".config" + DS + "TrickRig" + DS;
    return;
  }

  //## Открыть/создать файл базы данных с конфигурацией
  void config::open_sqlite()
  {
    std::string PathNameCfg = UserTrConfDir + "config.db";
    return;
  }

  //## Сохрание настроек
  void config::save(void)
  {
    return;
  }

  //## Установка соотношения сторон окна
  void config::set_size(int w, int h)
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
  int config::get_w(void) { return gui.w; }
  int config::get_h(void) { return gui.h; }
  std::string config::filepath(tr::FileDestination D) { return fp_name[D]; }

  //## Тестовая функция получения значения
  int config::get_value(void)
  {
    return value;
  }

} //namespace tr
