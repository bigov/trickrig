//============================================================================
//
// file: io.cpp
//
// Библиотека функций ввода-вывода
//
//============================================================================
#include <sys/stat.h>
#include "tools.hpp"

namespace tr
{

// Настройка пиксельных шрифтов
// ----------------------------
// "FontMap1" - однобайтовые символы
const std::string FontMap1 { "_'\"~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghi"
                               "jklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0"
                               "123456789 "};
// "FontMap2" - каждый символ занимает по два байта
const std::string FontMap2 { "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗ"
                               "ИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" };
uint FontMap1_len = 0; // значение будет присвоено в конструкторе класса

uint f_len = 160; // количество символов в текстуре шрифта
texture Font12n { "../assets/font_07x12_nr.png", f_len }; //шрифт 07х12 (норм)
texture Font15n { "../assets/font_08x15_nr.png", f_len }; //шрифт 08х15 (норм)
texture Font18n { "../assets/font_10x18_nr.png", f_len }; //шрифт 10x18 (норм)
texture Font18s { "../assets/font_10x18_sh.png", f_len }; //шрифт 10x18 (тень)
texture Font18l { "../assets/font_10x18_lt.png", f_len }; //шрифт 10x18 (светл)

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


bool operator== (const i3d& A, const i3d& B)
{
  return ((A.x == B.x) && (A.y == B.y) && (A.z == B.z));
}


bool operator== (const px &A, const px &B)
{
  return ((A.r == B.r) && (A.g == B.g) && (A.b == B.b) && (A.a == B.a));
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


  //## Генератор случайных положительных int
  int random_int()
  {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
     return std::rand();
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
  uchar int2uchar(int v)
  {
    if(v < 0)         return 0x00;
    else if (v > 255) return 0xFF;
    else return static_cast<uchar>(v);
  }

  ///
  /// \brief img::img
  /// \param width
  /// \param height
  ///
  image::image(ulong new_width, ulong new_height)
  {
     Data.resize(new_width * new_height);

    _width = static_cast<uint>(new_width);  // ширина изображения в пикселях
    _height = static_cast<uint>(new_height); // высота изображения в пикселях
    _cell_width = _width/columns; // ширина ячейки в пикселях
    _cell_height = _height/rows; // высота ячейки в пикселях
  }

  ///
  /// \brief img::img
  /// \param width
  /// \param height
  /// \param pixel
  ///
  image::image(ulong new_width, ulong new_height, const px &pixel)
  {
    Data.resize(new_width * new_height, pixel);
    columns = 1;      // число ячеек в строке (по-умолчанию)
    rows = 1;      // число строк (по-умолчанию)
    _width = static_cast<uint>(new_width);  // ширина изображения в пикселях
    _height = static_cast<uint>(new_height); // высота изображения в пикселях
    _cell_width = new_width/columns; // ширина ячейки в пикселях
    _cell_height = new_height/rows; // высота ячейки в пикселях
  }


  ///
  /// \brief image::image
  /// \param filename
  ///
  image::image(const std::string &filename)
  {
    load(filename);
  }


  ///
  /// \brief Конструктор c загрузкой данных из файла
  /// \param filename
  /// \param cols
  /// \param rows
  ///
  texture::texture(const std::string& filename, uint new_cols, uint new_rows)
  {
    columns = new_cols;
    rows = new_rows;
    load(filename);
  }


  ///
  /// \brief Установка размеров
  /// \param W
  /// \param H
  ///
  void image::resize(uint new_width, uint new_height)
  {
    _width = new_width;    // ширина изображения в пикселях
    _height = new_height;  // высота изображения в пикселях

    _cell_width = _width/columns; // ширина ячейки в пикселях
    _cell_height = _height/rows; // высота ячейки в пикселях

    Data.resize(_width * _height, {0x00, 0x00, 0x00, 0x00});
  }


  ///
  /// \brief img::clear
  ///
  void image::clear(void)
  {
    std::vector<px> empty {};
    Data.clear();
    Data.swap(empty);
    Data.resize(_width * _height, {0x00, 0x00, 0x00, 0x00});
  }


  ///
  /// \brief img::fill
  /// \param color
  ///
  void image::fill(const px& color)
  {
    std::vector<px> empty {};
    Data.clear();
    Data.swap(empty);
    Data.resize(_width * _height, color);
  }


  ///
  /// \brief img::uchar_data
  /// \return
  ///
  uchar* image::uchar_t(void) const
  {
    return reinterpret_cast<uchar*>(px_data());
  }


  ///
  /// \brief img::px_data
  /// \return
  ///
  px* image::px_data(void) const
  {
    return const_cast<px*>(Data.data());
  }


  ///
  /// \brief Загрузки избражения из .PNG файла
  ///
  /// \param filename
  ///
  void image::load(const std::string &fname)
  {
    png_image info;
    memset(&info, 0, sizeof info);

    info.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&info, fname.c_str())) ERR("Can't read PNG image file");
    info.format = PNG_FORMAT_RGBA;
    resize(info.width, info.height);

    if (!png_image_finish_read(&info, nullptr, uchar_t(), 0, nullptr ))
    {
      png_image_free(&info);
      ERR(info.message);
    }
  }

  ///
  /// \brief     Копирование указанного фрагмента текстуры в изображение
  ///
  /// \param C   номер ячейки в строке таблицы текстур
  /// \param R   номер строки таблицы текстур
  /// \param dst изображение-приемник
  /// \param X   координата пикселя приемника
  /// \param Y   координата пикселя приемника
  ///
  void image::copy(uint C, uint R, image& dst, ulong X, ulong Y) const
  {
    if(C >= columns) C = 0;
    if(R >= rows) R = 0;

    uint frag_w = _width / columns;   // ширина фрагмента в пикселях
    uint frag_h = _height / rows;   // высота фрагмента в пикселях
    uint frag_sz = frag_h * frag_w;  // число копируемых пикселей

    uint frag_i = C * frag_w + R * frag_h * _width; // индекс начала фрагмента

    auto dst_i = X + Y * dst._width;       // индекс начала в приемнике
    //UINT dst_max = dst.w * dst.h;     // число пикселей в приемнике

    uint i = 0;              // сумма скопированных пикселей
    while(i < frag_sz)
    {
      auto d = dst_i;        // текущий индекс приемника
      uint s = frag_i;       // текущий индекс источника
      auto l = d + frag_w;   // конец копируемой строки пикселей

      // В данной версии копируются только полностью непрозрачные пиксели
      while(d < l)
      {
        if(Data[s].a == 0xFF) dst.Data[d] = Data[s];
        ++d;
        ++s;
        ++i;
      }

      dst_i += dst._width; // переход на следующую строку приемника
      frag_i += _width;    // переход на следующую строку источника
    }
  }


  ///
  /// \brief Добавление текста из текстурного атласа
  ///
  /// \param текстура шрифта
  /// \param строка текста
  /// \param массив пикселей, в который добавляется текст
  /// \param х - координата
  /// \param y - координата
  ///
  void textstring_place(const image &FontImg, const std::string &TextString,
                     image& Dst, ulong x, ulong y)
  {
    #ifndef NDEBUG
    if(x > Dst._width - utf8_size(TextString) * FontImg._cell_width)
      ERR ("gui::add_text - X overflow");
    if(y > Dst._height - FontImg._cell_height)
      ERR ("gui::add_text - Y overflow");
    #endif

    uint row = 0;                        // номер строки в текстуре шрифта
    uint n = 0;                          // номер буквы в выводимой строке
    size_t text_size = TextString.size(); // число байт в строке

    for(size_t i = 0; i < text_size; ++i)
    {
      auto t = char_type(TextString[i]);
      if(t == SINGLE)
      {
        size_t col = FontMap1.find(TextString[i]);
        if(col == std::string::npos) col = 0;
        FontImg.copy(col, row, Dst, x + (n++) * FontImg._cell_width, y);
      }
      else if(t == UTF8_FIRST)
      {
        size_t col = FontMap2.find(TextString.substr(i,2));
        if(col == std::string::npos) col = 0;
        else col = FontMap1_len + col/2;
        FontImg.copy(col, row, Dst, x + (n++) * FontImg._cell_width, y);
      }
    }
  }


///
/// \brief cross
/// \param s
/// \return номер стороны, противоположной указанной в параметре
///
unsigned char side_opposite(unsigned char s)
{
  switch (s)
  {
    case SIDE_XP: return SIDE_XN;
    case SIDE_XN: return SIDE_XP;
    case SIDE_YP: return SIDE_YN;
    case SIDE_YN: return SIDE_YP;
    case SIDE_ZP: return SIDE_ZN;
    case SIDE_ZN: return SIDE_ZP;
    default: return UCHAR_MAX;
  }
}


///
/// \brief vox_buffer::i3d_near
/// \param P
/// \param s
/// \param l
/// \return
///
///  Координаты опорной точки соседнего вокса относительно указанной стороны
///
i3d i3d_near(const i3d& P, uchar side, int side_len)
{
  switch (side) {
    case SIDE_XP: return i3d{ P.x + side_len, P.y, P.z };
    case SIDE_XN: return i3d{ P.x - side_len, P.y, P.z };
    case SIDE_YP: return i3d{ P.x, P.y + side_len, P.z };
    case SIDE_YN: return i3d{ P.x, P.y - side_len, P.z };
    case SIDE_ZP: return i3d{ P.x, P.y, P.z + side_len };
    case SIDE_ZN: return i3d{ P.x, P.y, P.z - side_len };
    default:      return P;
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

} //namespace tr
