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

///
/// \brief dir_list
/// \param path
/// \return
///
/// \details Возвращает список директорий в формате полного пути в каталоге
///
std::list<std::string> dirs_list(const std::string &path)
{
  std::list<std::string> D {};

  for(auto& it: std::filesystem::directory_iterator(path))
    if (std::filesystem::is_directory(it))  D.push_back(it.path().string());

  return D;
}

bool operator== (const px &A, const px &B)
{
  return ((A.r == B.r) && (A.g==B.g) && (A.b==B.b) && (A.a==B.a));
}

  ///
  /// \brief char_type
  /// \param c
  /// \return тип символа
  ///
  /// \details Определение типа байта - это часть многобайтового символа,
  /// или это однобайтовый символ.
  ///
  CHAR_TYPE char_type(char c)
  {
    if (!(c&128))
    {                     // Если не 1 в восьмом бите,
      return SINGLE;      // то это не UTF8
    }
    else if(!(c&64))
    {                     // Если 10xxxxxx,
      return UTF8_SECOND; // то это второй бит UTF8
    }
    else if (!(c&32))
    {                     // Если 110xxxxx,
      return UTF8_FIRST;  // то это первый бит UTF8
    }
    return UTF8_ERR;      // трехбайтовое число
  }


  ///
  /// \brief utf8_letters
  /// \param Text
  /// \return число букв в строке UTF-8
  ///
  size_t utf8_size(const std::string &Text)
  {
    size_t letters = 0;
    size_t text_size = Text.size(); // число байт в строке
    for(size_t i = 0; i < text_size; ++i)
    {
      switch (char_type(Text[i]))
      {
        case SINGLE:
          ++letters;
          break;
        case UTF8_FIRST:
          ++letters;
          break;
        default:
          break;
      }
    }
    return letters;
  }


  ///
  /// \brief wstring2string
  /// \param w std::wstring
  /// \return std::string
  ///
  /// \details  Конвертер из std::wstring в std::string
  std::string wstring2string(const std::wstring &w)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(w);
  }


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


  ///
  /// \brief Вспомогательная функция для структуры "px"
  ///
  u_char int_to_uchar(int v)
  {
    if(v < 0)         return 0x00;
    else if (v > 255) return 0xFF;
    else return static_cast<u_char>(v);
  }

  ///
  /// \brief img::img
  /// \param width
  /// \param height
  ///
  img::img(u_long width, u_long height)
    : Data(width * height), n_cols(_c), n_rows(_r),
    w_cell(_wc), h_cell(_hc), w_summ(_w), h_summ(_h)
  {
    _w = static_cast<u_int>(width);  // ширина изображения в пикселях
    _h = static_cast<u_int>(height); // высота изображения в пикселях
    _c = 1;      // число ячеек в строке
    _r = 1;      // число строк
    _wc = _w/_c; // ширина ячейки в пикселях
    _hc = _h/_r; // высота ячейки в пикселях
  }

  ///
  /// \brief img::img
  /// \param width
  /// \param height
  /// \param pixel
  ///
  img::img(u_long width, u_long height, const px &pixel)
    : Data(width * height, pixel), n_cols(_c), n_rows(_r),
    w_cell(_wc), h_cell(_hc), w_summ(_w), h_summ(_h)
  {
    _c = 1;      // число ячеек в строке (по-умолчанию)
    _r = 1;      // число строк (по-умолчанию)
    _w = static_cast<u_int>(width);  // ширина изображения в пикселях
    _h = static_cast<u_int>(height); // высота изображения в пикселях
    _wc = _w/_c; // ширина ячейки в пикселях
    _hc = _h/_r; // высота ячейки в пикселях
  }

  ///
  /// \brief Конструктор c загрузкой данных из файла
  /// \param filename
  /// \param cols
  /// \param rows
  ///
  img::img(const std::string &filename, u_int cols, u_int rows)
    : Data(0),  n_cols(_c), n_rows(_r),
      w_cell(_wc), h_cell(_hc), w_summ(_w), h_summ(_h)
  {
    _c = cols;
    _r = rows;
    load(filename);
  }

  ///
  /// \brief Установка размеров
  /// \param W
  /// \param H
  ///
  void img::resize(u_int width, u_int height)
  {
    _w = width;  // ширина изображения в пикселях
    _h = height; // высота изображения в пикселях

    _wc = _w/_c; // ширина ячейки в пикселях
    _hc = _h/_r; // высота ячейки в пикселях
    Data.clear();
    Data.resize(_w * _h, {0x00, 0x00, 0x00, 0x00});
  }

  ///
  /// \brief img::uchar_data
  /// \return
  ///
  u_char* img::uchar(void) const
  {
    return reinterpret_cast<u_char*>(px_data());
  }

  ///
  /// \brief img::px_data
  /// \return
  ///
  px* img::px_data(void) const
  {
    return const_cast<px*>(Data.data());
  }

  ///
  /// \brief Загрузки избражения из .PNG файла
  ///
  /// В случае загрузки текстуры можно указать количество элементов в строке
  /// и число строк. По-умолчанию оба параметра устанавливаются равными 1.
  ///
  /// \param filename
  ///
  void img::load(const std::string &fname)
  {
    png_image info;
    memset(&info, 0, sizeof info);
    info.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&info, fname.c_str()))
      ERR("Can't read PNG image file");

    info.format = PNG_FORMAT_RGBA;

    resize(info.width, info.height);

    if (!png_image_finish_read(&info, nullptr, uchar(), 0, nullptr ))
    {
      png_image_free(&info);
      ERR(info.message);
    }
  }

  ///
  /// \brief     Копирование изображения
  ///
  /// \param C   номер ячейки в строке таблицы текстур
  /// \param R   номер строки таблицы текстур
  /// \param dst изображение-приемник
  /// \param X   координата пикселя приемника
  /// \param Y   координата пикселя приемника
  ///
  void img::copy(u_int C, u_int R, img& dst, u_long X, u_long Y) const
  {
    if(C >= n_cols) C = 0;
    if(R >= n_rows) R = 0;

    u_int frag_w = w_summ / n_cols;             // ширина фрагмента в пикселях
    u_int frag_h = h_summ / n_rows;             // высота фрагмента в пикселях
    u_int frag_sz = frag_h * frag_w;  // число копируемых пикселей

    u_int frag_i = C * frag_w + R * frag_h * w_summ; // индекс начала фрагмента

    auto dst_i = X + Y * dst.w_summ;       // индекс начала в приемнике
    //UINT dst_max = dst.w * dst.h;     // число пикселей в приемнике

    u_int i = 0;              // сумма скопированных пикселей
    while(i < frag_sz)
    {
      auto d = dst_i;        // текущий индекс приемника
      u_int s = frag_i;       // текущий индекс источника
      auto l = d + frag_w;   // конец копируемой строки пикселей

      // В данной версии копируются только полностью непрозрачные пиксели
      while(d < l)
      {
        if(Data[s].a == 0xFF) dst.Data[d] = Data[s];
        ++d;
        ++s;
        ++i;
      }

      dst_i += dst.w_summ; // переход на следующую строку приемника
      frag_i += w_summ;    // переход на следующую строку источника
    }
  }


  ///
  /// \brief read_chars_file
  /// \param FNname
  /// \param Buffer
  /// \details Чтение в буфер содержимого текстового файла
  ///
  std::unique_ptr<char[]> read_chars_file(const std::string& FileName)
  {
    // проверка наличия файла
    struct stat Buf;
    if (stat (FileName.c_str(), &Buf) != 0) ERR("Missing file: " + FileName);

    // открытие файла
    std::ifstream file(FileName, std::ios::in|std::ios::ate);
    file.exceptions(std::ios_base::badbit|std::ios_base::failbit);
    if (!file.is_open()) ERR("Can't open " + FileName);

    auto size = static_cast<long long>(file.tellg());
    if(size < 1) return nullptr;

    auto data_size = static_cast<size_t>(size) + 1;
    auto data = std::make_unique<char[]>(data_size);

    file.seekg(0, std::ios::beg);
    file.read(data.get(), size);
    file.close();

    data[data_size - 1] = '\0';
    return data;
  }

  //## Вывод текстовой информации
  void info(const std::string & info)
  {
    std::cout << info << std::endl;
  }

} //namespace tr
