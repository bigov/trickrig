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
  image::image(uint new_width, uint new_height)
  {
     Data.resize(new_width * new_height);

    width = new_width;  // ширина изображения в пикселях
    height = new_height; // высота изображения в пикселях
  }

  ///
  /// \brief img::img
  /// \param width
  /// \param height
  /// \param pixel
  ///
  image::image(ulong new_width, ulong new_height, const px& color = { 0xFF, 0xFF, 0xFF, 0xFF })
  {
    width = static_cast<uint>(new_width);  // ширина изображения в пикселях
    height = static_cast<uint>(new_height); // высота изображения в пикселях

    Data.resize(new_width * new_height, color);
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
  /// \brief Установка размеров
  /// \param W
  /// \param H
  ///
  void image::resize(uint new_width, uint new_height, px Color)
  {
    width = new_width;    // ширина изображения в пикселях
    height = new_height;  // высота изображения в пикселях
    Data.resize(width * height, Color);
  }


  ///
  /// \brief img::fill
  /// \param color
  ///
  void image::fill(const px& color)
  {
    Data.clear();
    Data.resize(width * height, color);
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

    width = info.width;    // ширина изображения в пикселях
    height = info.height;  // высота изображения в пикселях
    Data.resize(width * height, {0x00, 0x00, 0x00, 0x00});

    if (!png_image_finish_read(&info, nullptr, uchar_t(), 0, nullptr ))
    {
      png_image_free(&info);
      ERR(info.message);
    }
  }


  ///
  /// \brief     Копирование одного изображения в другое
  ///
  /// \param dst изображение-приемник
  /// \param X   координата пикселя приемника
  /// \param Y   координата пикселя приемника
  ///
  void image::put(image& dst, ulong x, ulong y) const
  {
    uint src_width = width;
    if( dst.width < width + x ) src_width = dst.width - x;
    uint src_height = height;
    if( dst.height < height + y ) src_height = dst.height - y;

    uint i_max = src_height * src_width;// число копируемых пикселей
    uint src_i = 0;                 // индекс начала фрагмента
    uint dst_i = x + y * dst.width; // индекс начала в приемнике
    uint i = 0;                     // сумма скопированных пикселей

    while(i < i_max)
    {
      uint s_row = src_i;           // текущий индекс источника
      uint d_row = dst_i;           // текущий индекс приемника
      uint d_max = d_row + src_width; // конец копируемой строки пикселей

      // В данной версии копируются только полностью непрозрачные пиксели
      while(d_row < d_max)
      {
        if(Data[s_row].a == 0xFF) dst.Data[d_row] = Data[s_row];
        ++d_row;
        ++s_row;
        ++i;
      }

      dst_i += dst.width; // переход на следующую строку приемника
      src_i += width;     // переход на следующую строку источника
    }
  }
/*

dst - нижнее изобрадение, scr - верхнее

out_a = src_a + dst_a * ( 1 - src_a );

1) out_c = (src_c * scr_a + dst_c * dst_a * ( 1 - src_a )) / out_a
   out_a == 0 -> out_c = 0
2) out_c = scr_c + dst_c * ( 1 - src_a );
*/

  inline float fl(const unsigned char c) { return static_cast<float>(c)/255.f; }

  px blend_1(px& src, px& dst)
  {
    uint r=0, g=1, b=2, a=3;

    float S[] = { fl(src.r), fl(src.g), fl(src.b), fl(src.a) };
    float D[] = { fl(dst.r), fl(dst.g), fl(dst.b), fl(dst.a) };
    float R[4];

    R[a] = S[a] + D[a] * ( 1.f - S[a] );
    if(R[a] > 0.f)
    {
      R[r] = ( S[r] * S[a] + D[r] * D[a] * ( 1.f - S[a])) / R[a];
      R[g] = ( S[g] * S[a] + D[g] * D[a] * ( 1.f - S[a])) / R[a];
      R[b] = ( S[b] * S[a] + D[b] * D[a] * ( 1.f - S[a])) / R[a];
    } else {
      R[r] = 0.f;
      R[g] = 0.f;
      R[b] = 0.f;
    }
    return { static_cast<uchar>(R[r] * 255),
             static_cast<uchar>(R[g] * 255),
             static_cast<uchar>(R[b] * 255),
             static_cast<uchar>(R[a] * 255)
    };
  }

  void image::paint_over(uint x, uint y, px* src_data, uint src_width, uint src_height)
  {
    auto original_width = src_width;
    if(src_width + x  > width  ) src_width  = width -  x;
    if(src_height + y > height ) src_height = height - y;

    uint i = 0;                          // число скопированных пикселей
    uint i_max = src_height * src_width; // сумма пикселей источника, которые надо скопировать
    uint src_row_start = 0;              // индекс в начале строки источника
    uint dst_row_start = x + y * width;  // индекс начального пикселя приемника

    while(i < i_max)
    {
      uint row_n = src_width;
      uint dst = dst_row_start;
      uint src = src_row_start;
      while(row_n > 0)
      {
        Data[dst] = blend_1( *(src_data + src), Data[dst]);
        dst += 1;
        src += 1;
        row_n -= 1;
        i += 1;
      }
      src_row_start += original_width; // переход на начало следующей строки источника
      dst_row_start += width;          // переход на начало следующей строки приемника
    }
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

    cell_width = width/columns; // ширина ячейки в пикселях
    cell_height = height/rows; // высота ячейки в пикселях

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
  void texture::put(uint C, uint R, image& dst, ulong X, ulong Y) const
  {
    if(C >= columns) C = 0;
    if(R >= rows) R = 0;

    uint frag_w = width / columns;   // ширина фрагмента в пикселях
    uint frag_h = height / rows;   // высота фрагмента в пикселях
    uint frag_sz = frag_h * frag_w;  // число копируемых пикселей

    uint frag_i = C * frag_w + R * frag_h * width; // индекс начала фрагмента

    auto dst_i = X + Y * dst.get_width();       // индекс начала в приемнике
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

      dst_i += dst.get_width(); // переход на следующую строку приемника
      frag_i += width;    // переход на следующую строку источника
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
  void textstring_place(const texture &FontImg, const std::string &TextString,
                     image& Dst, ulong x, ulong y)
  {
    #ifndef NDEBUG
    if(x > Dst.get_width() - utf8_size(TextString) * FontImg.get_cell_width())
      ERR ("gui::add_text - X overflow");
    if(y > Dst.get_height() - FontImg.get_cell_height())
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
        FontImg.put(col, row, Dst, x + (n++) * FontImg.get_cell_width(), y);
      }
      else if(t == UTF8_FIRST)
      {
        size_t col = FontMap2.find(TextString.substr(i,2));
        if(col == std::string::npos) col = 0;
        else col = FontMap1_len + col/2;
        FontImg.put(col, row, Dst, x + (n++) * FontImg.get_cell_width(), y);
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


  ///
  /// \brief load_font_file
  /// \param font_file_name
  ///
  std::unique_ptr<unsigned char[]> load_font_file(const char* font_file_name)
  {
    std::ifstream file (font_file_name, std::ifstream::binary);
    if (!file) fprintf(stderr, "error: not found font file");

    file.seekg (0, file.end);
    auto length = file.tellg();
    file.seekg (0, file.beg);
    std::vector<char> buffer(static_cast<size_t>(length), '\0');
    file.read (buffer.data(),length);
    if (!file) fprintf(stderr, "error: only %lld could be read", file.gcount());
    file.close();

    auto result = std::make_unique<unsigned char[]>(length);
    memmove(result.get(), buffer.data(), length);
    return result;
  }


  ///
  /// \brief string2unicode
  /// \param Text
  /// \return Unicode vector
  ///
  std::vector<int> string2unicode(std::string& Text)
  {
    std::vector<int> Result {};
    for(size_t i = 0; i < Text.size();)
    {
      unsigned int a=Text[i++];
      //if((a&0x80)==0);
      if((a&0xE0)==0xC0){
          a=(a&0x1F)<<6;
          a|=Text[i++]&0x3F;
      }else if((a&0xF0)==0xE0){
          a=(a&0xF)<<12;
          a|=(Text[i++]&0x3F)<<6;
          a|=Text[i++]&0x3F;
      }else if((a&0xF8)==0xF0){
          a=(a&0x7)<<18;
          a|=(a&0x3F)<<12;
          a|=(Text[i++]&0x3F)<<6;
          a|=Text[i++]&0x3F;
      }
      Result.push_back(a);
    }
    return Result;
  }

} //namespace tr
