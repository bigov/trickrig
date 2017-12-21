//===========================================================================
//
// file: main.cpp
//
// Запуск TrickRig
//
//===========================================================================
#include "config.hpp"
#include "main.hpp"
#include "io.hpp"
#include "scene.hpp"
#include "glfw.hpp"

namespace tr {
  tr::config Cfg = {};
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
