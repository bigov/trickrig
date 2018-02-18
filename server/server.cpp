#include <uv.h>
#include <iostream>
#include <vector>

int main() {

  std::vector<uv_loop_t> vL(sizeof(uv_loop_t));
  uv_loop_t *loop = vL.data();

  uv_loop_init(loop);
  std::cout << "\nNow HELLO!\n";
  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);

  return 0;

}
