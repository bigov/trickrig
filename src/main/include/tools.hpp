//============================================================================
//
// file: tools.hpp
//
//============================================================================
#ifndef TOOLS_HPP
#define TOOLS_HPP

#include "main.hpp"
#include "png.h"
#if not defined PNG_SIMPLIFIED_READ_SUPPORTED
  ERR("FAILURE: you must update the \"libpng\".");
#endif

namespace tr
{
enum CHAR_TYPE {
    SINGLE,
    UTF8_FIRST,
    UTF8_SECOND,
    UTF8_ERR
};

extern CHAR_TYPE char_type(char c);
extern size_t utf8_size(const std::string &Text);
extern std::string wstring2string(const std::wstring &w);

extern uchar int2uchar(int v); // Вспомогательная функция для структуры "px"

extern std::vector<int> string2unicode(std::string& Text);

extern int get_msec(void);   // число миллисекунд от начала суток.
extern int random_int(void);
extern std::list<std::string> dirs_list(const std::string &path);
extern unsigned char side_opposite(unsigned char s);
extern i3d i3d_near(const i3d& P, uchar side, int side_len);

extern std::unique_ptr<char[]> read_chars_file(const std::string &FileName);
extern std::unique_ptr<unsigned char[]> load_font_file(const char* font_file_name);

}

#endif //TOOLS_HPP
