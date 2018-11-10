//===========================================================================
//
// file: main.cpp
//
// Запуск TrickRig
//
//===========================================================================
#include "main.hpp"
#include "config.hpp"
#include "io.hpp"
#include "scene.hpp"
#include "glfw.hpp"
#include "dbwrap.hpp"

namespace tr {
  tr::camera_3d Eye = {};  // главная камера 3D вида
  tr::cfg TrConfig = {};   // настройка параметров
}

//##
int main()
{
#ifndef NDEBUG
  assert(sizeof(GLfloat) == 4);
  tr::info("\n -- Enable debug mode --\n");
#endif

  try
  {
    tr::TrConfig.load();     // Загрузка конфигурации
    tr::window_glfw Win {};  // Настройка OpenGL окна
    tr::scene Scene {};      // Сборка сцены
    Win.show(Scene);         // Цикл рендера
    tr::TrConfig.save();     // Сохранение конфигурации
  }
  catch(std::exception & e)
  {
    tr::info(e.what());
    return EXIT_FAILURE;
  }
  catch(...)
  {
    tr::info("FAILURE: undefined exception.");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

//## Установка значений по-умолчанию для параметров конфигурации
void tr::init_config_db(const std::string & fname)
{
/// Процедура вызывается при инициализации класса конфигурации,
/// в случае, если он не находит файл базы данных.

  tr::info("Init new database file.\n");
  tr::sqlw Db;
  Db.open(fname);
  for(auto &msg: Db.ErrorsList) tr::info(msg);
  std::string Q = "CREATE TABLE IF NOT EXISTS init ( \
                   rowid INTEGER PRIMARY KEY,        \
                   key INTEGER,                      \
                   usr TEXT DEFAULT '',              \
                   val TEXT);";

  char q[255];
  const char *tpl = "INSERT INTO init (key, val) VALUES (%d, '%s');";

  sprintf(q, tpl, TTF_FONT,           "DejaVuSansMono.ttf"); Q += q;
//  sprintf(q, tpl, PNG_HUD,            "hud.png");            Q += q;
  sprintf(q, tpl, PNG_TEXTURE0,       "tex0_512.png");       Q += q;
  sprintf(q, tpl, SHADER_VERT_SCENE,  "vert.glsl");          Q += q;
  sprintf(q, tpl, SHADER_GEOM_SCENE,  "geom.glsl");          Q += q;
  sprintf(q, tpl, SHADER_FRAG_SCENE,  "frag.glsl");          Q += q;
  sprintf(q, tpl, SHADER_VERT_SCREEN, "scr_vert.glsl");      Q += q;
  sprintf(q, tpl, SHADER_FRAG_SCREEN, "scr_frag.glsl");      Q += q;
  sprintf(q, tpl, DB_TPL_FNAME,       "surf_tpl.db");        Q += q;
  sprintf(q, tpl, WINDOW_SCREEN_FULL, "0");                  Q += q;
  sprintf(q, tpl, WINDOW_WIDTH,       "800");                Q += q;
  sprintf(q, tpl, WINDOW_HEIGHT,      "600");                Q += q;
  sprintf(q, tpl, WINDOW_TOP,         "50");                 Q += q;
  sprintf(q, tpl, WINDOW_LEFT,        "100");                Q += q;
  sprintf(q, tpl, VIEW_FROM_X,        "0.5");                Q += q;
  sprintf(q, tpl, VIEW_FROM_Y,        "2.0");                Q += q;
  sprintf(q, tpl, VIEW_FROM_Z,        "0.5");                Q += q;
  sprintf(q, tpl, LOOK_AZIM,          "0.0");                Q += q;
  sprintf(q, tpl, LOOK_TANG,          "0.0");                Q += q;

  Db.exec(Q.c_str());
  for(auto &msg: Db.ErrorsList) tr::info(msg);
  Db.close();
  return;
}

