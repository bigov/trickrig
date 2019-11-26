//===========================================================================
//
// file: main.cpp
//
// Запуск TrickRig
//
//===========================================================================

#include "gui.hpp"

namespace tr
{
  // Инициализация глобальных объектов
  std::string AppPathDir {};  // Абсолютный путь к исполняемому файлу приложения
}


///
/// \brief main
/// \return
///
int main(int, char* argv[])
{
  using namespace tr;

  fs::path p = argv[0];
  // Путь к папке исполняемого файла (со слэшем в конце)
  AppPathDir = fs::absolute(p).remove_filename().u8string();

#ifndef NDEBUG
  assert(sizeof(GLfloat) == 4);
  info("--- --- ---\nExec path: " + AppPathDir);
  info("Debug mode: ON\n--- --- ---\n");
#endif

  try
  {
    wglfw MainOpenGLContext {};
    if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    if(!gladLoadGL()) { ERR("FAILURE: can't load GLAD."); }

    gui AppGUI { &MainOpenGLContext };
    AppGUI.show();
  }
  catch(std::exception & e)
  {
    info(e.what());
    return EXIT_FAILURE;
  }
  catch(...)
  {
    info("FAILURE: undefined exception.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
