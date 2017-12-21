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
  struct GuiParams
  {
    int w = 0;
    int h = 0;
    float aspect = 1.0f;
  };

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

    public:
      cfg(void);
      ~cfg(void);

      glm::vec3 ViewFrom = {0.5f, 5.0f, 0.5f};

      static GuiParams gui;
      static std::unordered_map<int, std::string> InitApp;

      void load(void);
      void save(void);
      static void set_size(int w, int h);
      static int get_w(void);
      static int get_h(void);
      static std::string store(tr::ENUM_INIT);
  };

  extern cfg Cfg;

} //namespace

#endif
