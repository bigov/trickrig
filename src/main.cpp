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
  fs::path p = argv[0];
  // Путь к папке исполняемого файла (со слэшем в конце)
  tr::AppPathDir = fs::absolute(p).remove_filename().u8string();

#ifndef NDEBUG
  assert(sizeof(GLfloat) == 4);
  tr::info("\nDebug mode: ON");
  tr::info("Exec path: " + tr::AppPathDir);
#endif

  try
  {
    tr::wglfw Win {};     // Настройка OpenGL окна
    tr::scene Scene {};   // Сборка сцены
    Win.show(Scene);      // Цикл рендера
    tr::cfg::save_app();  // Сохранение конфигурации
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
