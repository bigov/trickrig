//============================================================================
//
// file: config.hpp
//
// Заголовок класса управления настройками приложения
//
//============================================================================
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <filesystem>
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

      static std::string UserDirDB;   // папка конфигов пользователя
      static v_str AppParams;         // общие настройки приложения
      static v_str MapParams;         // параметры сессии

      static void set_user_dir(void); // выбор пользовательской папки

    public:
      static db DataBase;
      static std::string CfgFname;  // конфиг текущей сесии
      static layout WinLayout;      // размер и положение главного окна

      static std::string DS;        // символ разделителя папок
      static std::string AssetsDir; // папка служебных файлов приложения

      static void map_view_load(const std::string& DirName, std::shared_ptr<glm::vec3> ViewFrom, float* look);
      static void map_view_save(std::shared_ptr<glm::vec3> View, float* look_dir);
      static void load(char** argv);
      static void save(const layout& WindowLayout);
      static std::string create_map(const std::string &MapName);
      static std::string app_key(APP_INIT);
      static std::string map_key(MAP_INIT);
      static std::string file_path_texture(APP_INIT);
      static std::string app_data_dir(void);
      static std::string map_name(const std::string &FolderName);
  };

} //namespace

#endif
