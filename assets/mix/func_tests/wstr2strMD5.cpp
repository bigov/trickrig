/*
 * Функция MD5wstring принимает широкую строку и возвращает MD5 код принятого
 * текста в стандартной строке.
 *
 * ВАЖНО: так как для вычисления MD5 используется openssl, то надо подключить
 * ее модули:
 *
 * \> pkg-config --libs openssl
 *
 * -LD:/MSYS2/mingw64/lib -lssl -lcrypto
 *
 *
 */

#include <iostream>
#include <iomanip>
#include <openssl/md5.h>

std::string MD5wstring(const std::wstring &wS )
{
  unsigned char d[MD5_DIGEST_LENGTH];
  unsigned char c[wS.length()];
  memcpy(c, wS.c_str(), wS.length());
  MD5(c, wS.length(), &d[0]);

  std::stringstream str;
  str.setf(std::ios_base::hex, std::ios::basefield);
  str.setf(std::ios_base::uppercase);
  str.fill('0');

  for(size_t i=0; i < MD5_DIGEST_LENGTH; ++i)
    str << std::setw(2) << static_cast<unsigned short>(d[i]);

  return str.str();
}

int main(int, char**)
{
  std::cout << MD5wstring( L"Й" ) << "\n";
  return 0;
}

