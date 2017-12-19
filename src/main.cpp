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
  tr::config Cfg = {};
}

//##
int main()
{
  try
  {
    #ifndef NDEBUG  // В режиме отладки загрузку конфигурации
    tr::Cfg.load(); // производим с выводом перехваченых сообщений
    #endif
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
