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

      // Запретить копирование и перенос
      cfg(const cfg&) = delete;
      cfg& operator=(const cfg&) = delete;
      cfg(cfg&&) = delete;
      cfg& operator=(cfg&&) = delete;

      static std::string DS;       // символ разделителя папок
      static std::string AssetsDir;   // папка служебных файлов приложения
      static std::string UserDir;     // папка конфигов пользователя
      static v_str AppParams;         // общие настройки приложения
      static v_str MapParams;         // параметры сессии

      static void set_user_dir(void); // выбор пользовательской папки

    public:
      static db DataBase;
      static std::string CfgFname; // конфиг текущей сесии
      static layout WinLayout;     // размер и положение главного окна

      static void map_view_load(const std::string &DirName, camera_3d &Eye);
      static void map_view_save(const camera_3d& Eye);
      static void load(char** argv);
      static void save(const layout& WindowLayout);
      static std::string create_map(const std::string &MapName);
      static std::string app_key(APP_INIT);
      static std::string map_key(MAP_INIT);
      static std::string user_dir(void);
      static std::string map_name(const std::string &FolderName);
  };

} //namespace

#endif
