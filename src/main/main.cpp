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
#include "app.hpp"
#include <time.h>


namespace tr
{
  // Инициализация глобальных объектов
  std::atomic<int> render_indices {0};
  std::atomic<int> click_side_vertex_id {0};
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
  std::streambuf* _cerr = std::cerr.rdbuf();
  std::streambuf* _clog = std::clog.rdbuf();

  char const err_fname[] = "tr_errs.txt"; // Errors log-file
  char const log_fname[] = "tr_logs.txt"; // Inform log-file

  std::ofstream tr_err_file(err_fname);
  std::cerr.rdbuf(tr_err_file.rdbuf()); // Redirect std::cerr in file
  std::ofstream tr_log_file(log_fname);
  std::clog.rdbuf(tr_log_file.rdbuf()); // Redirect std::clog in file

  time_t rawtime {}; time(&rawtime);
  std::clog << ctime(&rawtime);         // Current time

  using namespace tr;
  try
  {
    cfg::load(argv); // загрузка конфигурации
    app MyApp {};   // Создать окно приложения
    MyApp.show();   // Главный цикл
  }
  catch(std::exception & e)
  {
    std::cerr << e.what();
    std::cerr.rdbuf(_cerr);  // restore System std::cerr
    std::clog.rdbuf(_clog);  // restore System std::clog
    return EXIT_FAILURE;
  }
  catch(...)
  {
    std::cerr << "FAILURE: undefined exception";
    std::cerr.rdbuf(_cerr);  // restore System std::cerr
    std::clog.rdbuf(_clog);  // restore System std::clog
    return EXIT_FAILURE;
  }

  log_mtx.lock();
  std::clog << "TrickRig exit success" << std::endl;
  log_mtx.unlock();

  std::cerr.rdbuf(_cerr);  // restore System std::cerr
  std::clog.rdbuf(_clog);  // restore System std::clog
  return EXIT_SUCCESS;
}
