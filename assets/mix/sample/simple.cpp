#include <iostream>
#include "glad.h"
#include <GLFW/glfw3.h>

#define ERR throw std::runtime_error
/*

Intel: создает окно по 4.2-core включительно
Профиль compatibility не создается ни с одной из весий.

*/

///
/// \brief app
///
void app(void)
{
  if(!glfwInit()) ERR("Error init GLFW lib.");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  auto win_ptr = glfwCreateWindow(640, 480, "test", nullptr, nullptr);
  if (nullptr == win_ptr) ERR("Creating Window fail\n");
  glfwMakeContextCurrent(win_ptr);
  glfwSwapInterval(0);
  if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) ERR("FAILURE: can't load GLAD.");



  int k;
  std::cin >> k;
  glfwTerminate();

}

///
/// \brief main
/// \return
///
int main(int, char**)
{
  try
  {
    app();
  }
  catch(std::exception & e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch(...)
  {
    std::cout << "FAILURE: undefined exception.\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
