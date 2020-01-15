//===========================================================================
//
// file: main.cpp
//
// GL_TEXTURE0 - текстура поверхности воксов
// GL_TEXTURE1 - текстура для рендера 3D сцены
// GL_TEXTURE2 - рендер идентификации примитивов 3D сцены (цвет = id примитива)
//             - текстура GUI в режиме "меню"
//             - текстура HUD в 3D режиме
//
//===========================================================================

#include "gui.hpp"

namespace tr
{
  // Инициализация глобальных объектов
  std::atomic<int> render_indices = 0;
  std::mutex mutex_viewfrom {}; // Доступ к положению камеры
  std::mutex VboAccess {};      // Доступ к буферу вершин
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
    cfg::load(argv); // загрузка конфигурации
    gui AppGUI {};   // Создать окно приложения
    AppGUI.show();   // Главный цикл
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
