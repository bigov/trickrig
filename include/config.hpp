//============================================================================
//
// file: config.hpp
//
// Заголовок класса управления настройками приложения
//
//============================================================================
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include "main.hpp"
#include "io.hpp"
#include "sqlw.hpp"

namespace tr
{
  class cfg
  {
    private:
      tr::sqlw SqlDb = {};
      std::string UserDir    = "";           // папка конфигов пользователя
      std::string UserConfig = "config.db";  // конфиг пользователя
      std::string DS = "";                   // символ разделителя папок
      static std::string AssetsDir;          // папка конфигов пользователя

      cfg(const tr::cfg &)            = delete;
      cfg& operator=(const tr::cfg &) = delete;
      void set_user_conf_dir(void); // выбор пользовательской папки
      static std::unordered_map<int, std::string> InitParams;

    public:
      cfg(void);
      ~cfg(void);

      void load(void);
      void save(void);
      static std::string get(tr::ENUM_INIT);
  };

  extern cfg Cfg;

} //namespace

#endif
