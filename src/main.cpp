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
  win_data WinData     {};  // параметры окна приложения
  camera_3d Eye          {};  // главная камера 3D вида
  wglfw GLWindow {};
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
    WinData.layout_set(cfg::WinLayout);

    GLWindow.init(WinData.Layout.width, WinData.Layout.height,
                  WinData.minwidth, WinData.minheight,
                  WinData.Layout.left, WinData.Layout.top);
    GLWindow.set_error_observer(WinData);    // отслеживание ошибок
    GLWindow.set_cursor_observer(WinData);   // курсор мыши в окне
    GLWindow.set_button_observer(WinData);   // кнопки мыши
    GLWindow.set_keyboard_observer(WinData); // клавиши клавиатуры
    GLWindow.set_position_observer(WinData); // положение окна
    GLWindow.add_size_observer(WinData);     // размер окна
    GLWindow.set_close_observer(WinData);    // закрытие окна

    MatProjection = glm::perspective(1.118f, WinData.aspect, zNear, zFar);

    gui AppGUI {};
    AppGUI.show();

    cfg::save(WinData.Layout); // Сохранение положения окна
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
