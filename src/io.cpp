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
  //## Переворачивает рисунок по вертикали копируя построчно
  void image::flip_vert(void)
  {
    auto tmp = Data;
    size_t width = Data.size()/h; // длина строки в байтах
    size_t i = 0;
    for (size_t row = h - 1; row > 0; row--)
    {
      memcpy(&Data[i], &tmp[width * row], width);
      i += width;
    }
    return;
  }

  //## Для обеспечения работы контейнера map c ключем f2d
  bool operator< (tr::f2d const& left, tr::f2d const& right)
  {
    if (left.x != right.x) return left.x < right.x;
    else return left.z < right.z;
  }

  /*
  //## Для обеспечения работы контейнера map c ключем 3d
  bool operator< (tr::f3d const& left, tr::f3d const& right)
  {
    if (left.y != right.y)      {return left.y < right.y;}
    else if (left.z != right.z) {return left.z < right.z;}
    else                        {return left.x < right.x;}
  }
  */

  //## Для обеспечения работы контейнера map c ключем i3d
  bool operator< (tr::i3d const& left, tr::i3d const& right)
  {
    if (left.y != right.y)      {return left.y < right.y;}
    else if (left.z != right.z) {return left.z < right.z;}
    else                        {return left.x < right.x;}
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
  image get_png_img(const std::string & f_n)
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
    image res {};
    res.w = static_cast<GLsizei>(info.width);
    res.h = static_cast<GLsizei>(info.height);
    res.size = PNG_IMAGE_SIZE(info);
    res.Data.assign(res.size, '\0');

    if (!png_image_finish_read(&info, nullptr, res.Data.data(), 0, nullptr ))
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

  //## Чтение содержимого текстового файла в буфер std::vector<char>
  void read_chars_file(const std::string &FNname, std::vector<char> &Buffer)
  {
    // проверка наличия файла
    struct stat Buf;
    if (stat (FNname.c_str(), &Buf) != 0) ERR("Missing file: " + FNname);
    // чтение файла
    std::ifstream file(FNname, std::ios::in|std::ios::ate);
    file.exceptions(std::ios_base::badbit|std::ios_base::failbit);
    if (!file.is_open()) ERR("Can't open " + FNname);
    auto size = static_cast<long long>(file.tellg());
    Buffer.clear();
    Buffer.resize(static_cast<size_t>(size + 1), '\0');
    file.seekg(0, std::ios::beg);
    file.read(Buffer.data(), size);
    file.close();
    return;
  }

  //## Вывод текстовой информацияя информации
  void info(const std::string & info)
  {
    std::cout << info << std::endl;
    return;
  }

} //namespace tr
