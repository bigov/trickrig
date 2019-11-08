//============================================================================
//
// file: config.hpp
//
// Заголовок класса управления настройками приложения
//
//============================================================================
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "db.hpp"

namespace tr
{
  class cfg
  {
    private:
      cfg(void)                   = delete;
      cfg(const cfg &)            = delete;
      cfg& operator=(const cfg &) = delete;

      static std::string AssetsDir;   // папка служебных файлов приложения
      static std::string UserDir;     // папка конфигов пользователя
      static v_str AppParams;         // общие настройки приложения
      static v_str MapParams;         // параметры сессии

      static void set_user_dir(void); // выбор пользовательской папки

    public:
      static db DataBase;
      static std::string DS;          // символ разделителя папок
      static std::string CfgFname;    // конфиг текущей сесии

      static void load_map_cfg(const std::string &DirName);
      static void load_app_cfg(void);
      static void save_map_view(void);
      static void save_app(void);
      static std::string create_map(const std::string &MapName);
      static std::string app_key(APP_INIT);
      static std::string map_key(MAP_INIT);
      static std::string user_dir(void);
      static std::string map_name(const std::string &FolderName);
  };

} //namespace

#endif
