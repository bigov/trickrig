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


///
/// \brief operator ==
/// \param A
/// \param B
/// \return
///
bool operator== (const i3d& A, const i3d& B)
{
  return ((A.x == B.x) && (A.y == B.y) && (A.z == B.z));
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
