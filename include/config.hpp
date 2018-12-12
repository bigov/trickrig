//============================================================================
//
// file: config.hpp
//
// Заголовок класса управления настройками приложения
//
//============================================================================
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "main.hpp"
#include "io.hpp"
#include "dbwrap.hpp"

namespace tr
{
  class cfg
  {
    private:
      cfg(void)                       = delete;
      cfg(const tr::cfg &)            = delete;
      cfg& operator=(const tr::cfg &) = delete;

      static tr::sqlw SqlDb;
      static std::string UserDir;   // папка конфигов пользователя
      static std::string DS;        // символ разделителя папок
      static std::string AssetsDir; // папка конфигов пользователя
      static std::unordered_map<int, std::string> InitParams;
      static void check_user_dir(void);   // выбор пользовательской папки

    public:
      static std::string RunName;  // имя папки данных выбранного района
      static std::string CfgFname; // конфиг пользователя
      static void load(void);
      static void save(void);
      static std::string get(tr::ENUM_INIT);
  };

} //namespace

#endif
