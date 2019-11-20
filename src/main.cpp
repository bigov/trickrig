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
  glm::mat4 MatProjection {}; // матрица проекции 3D сцены
  float zNear = 1.f;          // расстояние до ближней плоскости матрицы проекции
  float zFar  = 10000.f;      // расстояние до дальней плоскости матрицы проекции

  std::string AppPathDir {};  // Абсолютный путь к исполняемому файлу приложения
  win_data AppWindow     {};  // параметры окна приложения
  camera_3d Eye          {};  // главная камера 3D вида
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
    cfg::load();
    AppWindow.layout_set(cfg::WinLayout);
    MatProjection = glm::perspective(1.118f, AppWindow.aspect, zNear, zFar);

    gui AppGUI {};
    AppGUI.show();

    cfg::save(AppWindow.Layout); // Сохранение положения окна
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
