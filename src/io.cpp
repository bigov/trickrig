//============================================================================
//
// file: io.cpp
//
// Библиотека функций ввода-вывода
//
//============================================================================
#include <sys/stat.h>
#include "io.hpp"

namespace tr
{

  //## Для обеспечения работы контейнера map c ключем f2d
  bool operator< (f2d const& left, f2d const& right)
  {
    if (left.x != right.x) return left.x < right.x;
    else return left.z < right.z;
  }


  //## Для обеспечения работы контейнера map c ключем 3d
  bool operator< (f3d const& left, f3d const& right)
  {
    if (left.x != right.x)      {return left.x < right.x;}
    else if (left.y != right.y) {return left.y < right.y;}
    else                        {return left.z < right.z;}
  }

  bool operator< (c3d const& left, c3d const& right)
  {
    if (left.x != right.x)      {return left.x < right.x;}
    else if (left.y != right.y) {return left.y < right.y;}
    else                        {return left.z < right.z;}
  }

  //## Генератор случайных положительных int
  int random_int()
  {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
     return std::rand();
  }

  //## Генератор случайных положительных short
  short random_short()
  {
    return
      static_cast<short>(random_int() % std::numeric_limits<short>::max());
  }

  //## Вычисляет число миллисекунд от начала суток
  int get_msec(void)
  {
    std::chrono::milliseconds ms =
      std::chrono::duration_cast< std::chrono::milliseconds >
      (std::chrono::system_clock::now().time_since_epoch());
    int k = 1000 * 60 * 60 * 24; // миллисекунд в сутках
    return static_cast<int>(ms.count() % k);
  }

  //## Чтение файла в формате PNG
  pngImg get_png_img(const std::string & f_n)
  {
    const char * filename = f_n.c_str();
    #if not defined PNG_SIMPLIFIED_READ_SUPPORTED
      ERR("FAILURE: you must update the \"libpng\".");
    #endif

    png_image info;
    memset(&info, 0, sizeof info);
    info.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&info, filename))
      ERR("Can't read PNG image file");

    info.format = PNG_FORMAT_RGBA;
    pngImg res {};
    res.w = static_cast<GLsizei>(info.width);
    res.h = static_cast<GLsizei>(info.height);
    res.size = PNG_IMAGE_SIZE(info);
    res.img.assign(res.size, '\0');

    if (!png_image_finish_read(&info, NULL, res.img.data(), 0, NULL ))
    {
      png_image_free(&info);
      ERR(info.message);
    }
    
    return res;
  }

  //## Преобразователь типов
  //
  // Упаковывает два коротких в одно целое. Используется для формирования
  // массива текстур в контенте OpenGL
  //
  int sh2int(short sh1, short sh2)
  {
    short sh_T[2] = {sh1, sh2};
    int * int_T = reinterpret_cast<int*>(sh_T);
    return *int_T;
  }

  //## Чтение содержимого текстового файла
  //
  std::vector<char> get_txt_chars(const std::string & fname)
  {
    // проверка наличия файла
    struct stat buffer;
    if (stat (fname.c_str(), &buffer) != 0) ERR("Missing file: " + fname);
      // чтение файла
    std::ifstream file(fname, std::ios::in|std::ios::ate);
    file.exceptions(std::ios_base::badbit|std::ios_base::failbit);
    if (!file.is_open()) ERR("Can't open " + fname);
    auto size = static_cast<long long>(file.tellg());
    std::vector<char> content (static_cast<size_t>(size + 1), '\0');
    file.seekg(0, std::ios::beg);
    file.read(content.data(), size);
    file.close();
    return content;
  }

  //## Вывод текстовой информацияя информации
  //
  void info(const std::string & info)
  {
    std::cout << info << std::endl;
    return;
  }

} //namespace tr
