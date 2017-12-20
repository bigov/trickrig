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
#include "sqlitew.hpp"

namespace tr
{
  struct GuiParams
  {
    int w = 0;
    int h = 0;
    float aspect = 1.0f;
  };

  enum FileDestination {
    FONT_FNAME,
    TEXTURE_FNAME,
    HUD_FNAME,
    SHADER_VERT_SCENE,
    SHADER_GEOM_SCENE,
    SHADER_FRAG_SCENE,
    SHADER_VERT_SCREEN,
    SHADER_FRAG_SCREEN,
  };

  class config
  {
    private:
      sqlite3 *db = nullptr;
      std::string UserTrConfDir = "";  // папка конфигов пользователя
      std::string DS = "";             // символ разделителя папок

      config(const tr::config &)            = delete;
      config& operator=(const tr::config &) = delete;
      int value = 0;
      void open_sqlite(void);
      void set_user_conf_dir(void); // выбор пользовательской папки

    public:
      config(void);
      ~config(void);

      glm::vec3 ViewFrom = {0.5f, 5.0f, 0.5f};

      static GuiParams gui;
      static std::unordered_map<tr::FileDestination, std::string> fp_name;

      void load(void);
      void save(void);
      int get_value(void);
      static void set_size(int w, int h);
      static int get_w(void);
      static int get_h(void);
      static std::string filepath(tr::FileDestination);
  };

  extern config Cfg;

} //namespace

#endif
