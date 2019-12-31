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
  std::atomic<int> render_indices = 0;
  std::mutex mutex_viewfrom {}; // Доступ к положению камеры
  std::mutex mutex_voxes_db {}; // разделение доступа к буферу вершин
}


///
/// \brief main
/// \return
///
int main(int, char* argv[])
{
  using namespace tr;

  try
  {
    cfg::load(argv);                    // Начальная загрузка конфигурации
    wglfw MainOpenGLContext {};         // Создать OpenGL контекст
    gui AppGUI { &MainOpenGLContext };  // Создать окно приложения
    AppGUI.show();                      // Главный цикл
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
