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
 /* В сборке с отладкой вызов процедуры загрузки конфига
  * производится из главного модуля с перехватом и выводом
  * сообщений об ошибках в случае возникновения прерываний.
  *
  * В релизе загрузка конфигурации вызывается сразу без
  * поддержки возможности вывода сообщений.
  */
#ifdef NDEBUG
  load();
#else
  assert(sizeof(GLfloat) == 4);
  tr::info("\n -- Debug mode is enabled. --\n");
#endif
  set_user_conf_dir();

  return;
}
  //## Деструктор класса
  config::~config(void)
  {
    sqlite3_close(db);
    return;
  }

  //## Загрузка конфигурации приложения
  void config::load(void)
  {
    open_sqlite();

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

  //## Вывод результата запроса к базе данных
  //                             сумма строк |  значения  |    заголовки    |
  static int callback(void *NotUsed, int argc, char **argv, char **azColName)
  {
 /* Arguments:
  *
  *   NotUsed - Ignored in this case, see the documentation for sqlite3_exec
  *      argc - количество значений (колонок) в каждой строке результата
  *      argv - значения
  * azColName - имена колонок
  */

    if(argc == 0) info("no table present\n");
    else info("table present\n");

    for(int i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    if(NotUsed != nullptr) return 0;
    return 0;
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
