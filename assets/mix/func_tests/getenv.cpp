#include <iostream>
#include <cstdlib>
#include <vector>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;

#ifdef __cplusplus
extern "C" {
#endif

#include <sec_api/stdlib_s.h>
errno_t getenv_s(
    size_t     *ret_required_buf_size,
    char       *buf,
    size_t      buf_size_in_bytes,
    const char *name
);
#ifdef __cplusplus
extern "C++" {
  template <size_t size>
  getenv_s(
      size_t *ret_required_buf_size,
      char (&buf)[size],
      const char *name
  ) { return getenv_s(ret_required_buf_size, buf, size, name); }
}
#endif
#ifdef __cplusplus
}
#endif


int main(int argc, char* argv[])
{
  char env_key[] = "USERPROFILE";
  size_t requiredSize;
  getenv_s( &requiredSize, NULL, 0, env_key);
  std::vector<char> libvar {};
  libvar.resize(requiredSize);
  getenv_s( &requiredSize, libvar.data(), requiredSize, env_key );
  std::string UserDir = std::string(libvar.data()) + std::string("\\AppData\\Roaming");
  std::cout << UserDir.c_str();
  std::cout << "\nCurrent path is " << fs::current_path() << '\n';

  fs::path p = argv[0];
  std::cout << "\nAbsolute path is " << fs::absolute(p).remove_filename() << '\n';

  return 0;
}

