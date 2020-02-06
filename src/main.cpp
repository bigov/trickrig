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

#include <iostream>
#include <fstream>
#include <string>
#include "gui.hpp"

namespace tr
{
  // Инициализация глобальных объектов
  std::atomic<int> render_indices = 0;
  std::mutex view_mtx {};  // Доступ к положению камеры
  std::mutex vbo_mtx {};   // Доступ к буферу вершин
  std::mutex log_mtx {};   // Журналирование
}


///
/// \brief main
/// \return
///
int main(int, char* argv[])
{
  std::streambuf* _cerr = std::cerr.rdbuf(); // Save System std::cerr
  std::streambuf* _clog = std::clog.rdbuf(); // Save System std::clog

  std::ofstream tr_err_file("tr_errs.txt");  // Errors log-file
  std::cerr.rdbuf(tr_err_file.rdbuf());      // Redirect std::cerr in file
  std::ofstream tr_log_file("tr_logs.txt");  // Inform log-file
  std::clog.rdbuf(tr_log_file.rdbuf());      // Redirect std::clog in file

  std::clog << "Start TrickRig" << std::endl;

  using namespace tr;
  try
  {
    cfg::load(argv); // загрузка конфигурации
    gui AppGUI {};   // Создать окно приложения
    AppGUI.show();   // Главный цикл
  }
  catch(std::exception & e)
  {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }
  catch(...)
  {
    std::cerr << "FAILURE: undefined exception";
    return EXIT_FAILURE;
  }

  log_mtx.lock();
  std::clog << "TrickRig exit success" << std::endl;
  log_mtx.unlock();

  //std::this_thread::sleep_for(std::chrono::seconds(2));

  std::cerr.rdbuf(_cerr);  // restore System std::cerr
  std::clog.rdbuf(_clog);  // restore System std::clog

  return EXIT_SUCCESS;
}
