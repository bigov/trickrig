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
#include "sqlw.hpp"

namespace tr {
  tr::cfg Cfg = {};
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
    tr::Cfg.load();           // Загрузка конфигурации
    tr::window_glfw Win = {}; // Настройка OpenGL окна
    tr::scene Scene = {};     // Сборка сцены
    Win.show(Scene);          // Цикл рендера
    tr::Cfg.save();           // Сохранение конфигурации
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

//## Создание и начальное заполнение базы данных конфигурации
void tr::init_config_db(const std::string & fname)
{
  tr::sqlw Db;
  Db.open(fname);
  for(auto &msg: Db.ErrorsList) tr::info(msg);
  Db.exec(
    "CREATE TABLE IF NOT EXISTS init ( \
     rowid INTEGER PRIMARY KEY,        \
     key INTEGER,                      \
     usr TEXT DEFAULT '',              \
     val TEXT                          \
     );");
  for(auto &msg: Db.ErrorsList) tr::info(msg);

  char query [256];
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    TTF_FONT,           "DejaVuSansMono.ttf"); Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    PNG_HUD,            "hud.png");            Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    PNG_TEXTURE0,       "tex0_512.png");       Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    SHADER_VERT_SCENE,  "vert.glsl");          Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    SHADER_GEOM_SCENE,  "geom.glsl");          Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    SHADER_FRAG_SCENE,  "frag.glsl");          Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    SHADER_VERT_SCREEN, "scr_vert.glsl");      Db.exec(query);
  sprintf(query,"REPLACE INTO init(key, val) VALUES(%d, '%s');",
    SHADER_FRAG_SCREEN, "scr_frag.glsl");      Db.exec(query);

  for(auto &msg: Db.ErrorsList) tr::info(msg);
  Db.close();
  return;
}

