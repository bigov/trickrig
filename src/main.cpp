//============================================================================
//
// file: main.cpp
//
// Процедура входа по-умолчанию для запуска приложения
//
//============================================================================
#include "config.hpp"
#include "main.hpp"
#include "io.hpp"
#include "scene.hpp"
#include "glfw.hpp"

namespace tr {
  tr::config Cfg {};
}

//##
int main()
{
  try
  {
    tr::Cfg.load();         // загрузка опций
    tr::window_glfw Win {}; // настройка OpenGL окна
    tr::scene Scene {};     // сборка сцены
    Win.show(Scene);        // цикл рендера
    tr::Cfg.save();         // сохранение конфигурации
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
