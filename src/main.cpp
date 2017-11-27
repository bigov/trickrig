//============================================================================
//
// file: main.cpp
//
// Процедура входа по-умолчанию для запуска приложения
//
//============================================================================
#include "main.hpp"
#include "io.hpp"
#include "space.hpp"
#include "glfw.hpp"

//##
int main()
{
  try
  {
    tr::Config cfg = {};
    cfg.load();
    tr::WindowGLFW win(&cfg);
    tr::Scene scene(&cfg);
    win.show(scene);
    cfg.save();
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
