//===========================================================================
//
// file: main.cpp
//
// Запуск TrickRig
//
//===========================================================================

//#include "wglfw.hpp"
#include "gui.hpp"

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
  tr::info("--- --- ---\nExec path: " + tr::AppPathDir);
  tr::info("Debug mode: ON\n--- --- ---\n");
#endif

  try
  {
    tr::cfg::load_app_cfg();

    //tr::wglfw Win {};    // Создать OpenGL окно
    //Win.show();          // Цикл рендера

    //std::unique_ptr<tr::gui> AppGUI = nullptr;   // Сборка сцены
    auto AppGUI = std::make_unique<tr::gui>();
    AppGUI->show();

    tr::cfg::save_app(); // Сохранение конфигурации
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
