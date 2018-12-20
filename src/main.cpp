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
#include "wglfw.hpp"

std::string tr::AppPathDir {};

///
/// \brief main
/// \return
///
int main(int, char* argv[])
{
#ifndef NDEBUG
  assert(sizeof(GLfloat) == 4);
  tr::info("\n -- Enable debug mode --\n");
#endif

  fs::path p = argv[0];
  tr::AppPathDir = fs::absolute(p).remove_filename().u8string();

  try
  {
    tr::wglfw Win {};     // Настройка OpenGL окна
    tr::scene Scene {};   // Сборка сцены
    Win.show(Scene);      // Цикл рендера
    tr::cfg::save();      // Сохранение конфигурации
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
