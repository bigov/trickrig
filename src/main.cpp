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
  std::string AppPathDir {}; // Абсолютный путь к исполняемому файлу приложения
  std::atomic<int> render_indices = 0;

  f3d MovingDist {};         // Вектор смещения между кадрами
  std::mutex mutex_mdist;
  std::mutex mutex_voxes_db; // разделение доступа к буферу вершин
  std::mutex mutex_vbo;      // разделение доступа к VBO
  std::mutex mutex_loading;  // ожидание загрузки сцены из базы данных в GPU
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
  AppPathDir = fs::absolute(p).remove_filename().string();

#ifndef NDEBUG
  assert(sizeof(GLfloat) == 4);
  info("--- --- ---\nExec path: " + AppPathDir);
  info("Debug mode: ON\n--- --- ---\n");
#endif

  try
  {
    wglfw MainOpenGLContext {};
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
