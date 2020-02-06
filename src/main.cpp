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

std::ofstream tr_err("tr_errs.txt");             // errors log-file
std::ofstream tr_log("tr_logs.txt");             // inform log-file
std::streambuf* sys_cerrbuf = std::cerr.rdbuf(); // save System std::cerr
std::streambuf* sys_clogbuf = std::clog.rdbuf(); // save System std::clog

void redirect_streambuf(void)
{
  std::cerr.rdbuf(tr_err.rdbuf()); // redirect std::cerr to err.txt
  std::clog.rdbuf(tr_log.rdbuf()); // redirect std::clog to log.txt
}

void restore_streambuf(void)
{
  std::cerr.rdbuf(sys_cerrbuf);    // restore System std::cerr
  std::clog.rdbuf(sys_clogbuf);    // restore System std::clog
}


namespace tr
{
  // Инициализация глобальных объектов
  std::atomic<int> render_indices = 0;
  std::mutex view_mtx {}; // Доступ к положению камеры
  std::mutex vbo_mtx {};      // Доступ к буферу вершин
}


///
/// \brief main
/// \return
///
int main(int, char* argv[])
{
  redirect_streambuf();
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
    std::cerr << "FAILURE: undefined exception.";
    return EXIT_FAILURE;
  }

  std::clog << "TrickRig normal completed." << std::endl;
  restore_streambuf();
  return EXIT_SUCCESS;
}
