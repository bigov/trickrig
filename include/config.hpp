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
#include "db.hpp"
//#include "rig.hpp"

namespace tr
{
  class cfg
  {
    private:
      cfg(void)                   = delete;
      cfg(const cfg &)            = delete;
      cfg& operator=(const cfg &) = delete;

      static db DataBase;
      static std::string AssetsDir;   // папка служебных файлов приложения
      static std::string UserDir;     // папка конфигов пользователя
      static v_str AppParams;
      static v_str MapParams;

      static void set_user_dir(void); // выбор пользовательской папки

    public:
      static std::string DS;          // символ разделителя папок
      static std::string CfgFname;    // конфиг текущей сесии

      static void load_map(const std::string &DirName);
      static void load_app_params(void);
      static void save(void);
      static void create_map(const std::string &MapName);
      static std::string app_key(APP_INIT);
      static std::string map_key(MAP_INIT);
      static std::string user_dir(void);
      static std::string map_name(const std::string &FolderName);
  };

} //namespace

#endif
