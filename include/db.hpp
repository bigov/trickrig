/*
 * file: db.hpp
 *
 * Заголовочный файл класса управления базой данных
 *
 */

#ifndef DB_HPP
#define DB_HPP

#include "wsql.hpp"
#include "voxel.hpp"
#include "area.hpp"
#include "framebuf.hpp"

namespace tr {


struct texture_coord {float u=0.0f, v=0.0f;};

// Параметры и режимы окна приложения
struct main_window {
  u_int width = 400;                    // ширина окна
  u_int height = 400;                   // высота окна
  u_int left = 0;                       // положение окна по горизонтали
  u_int top = 0;                        // положение окна по вертикали
  u_int btn_w = 120;                    // ширина кнопки GUI
  u_int btn_h = 36;                     // высота кнопки GUI
  u_int minwidth = (btn_w + 16) * 4;    // минимально допустимая ширина окна
  u_int minheight = btn_h * 4 + 8;      // минимально допустимая высота окна
  std::string* pInputBuffer = nullptr;  // строка ввода пользователя
  std::unique_ptr<frame_buffer> RenderBuffer = nullptr;    // рендер-буфер окна
  img* pWinGui = nullptr;               // текстура GUI окна

  bool run     = true;  // индикатор закрытия окна
  float aspect = 1.0f;  // соотношение размеров окна
  //bool resized = true;  // флаг наличия изменений параметров окна
  double xpos = 0.0;    // позиция указателя относительно левой границы
  double ypos = 0.0;    // позиция указателя относительно верхней границы
  int fps = 120;        // частота кадров (для коррекции скорости движения)
  glm::vec3 Cursor = { 200.5f, 200.5f, .0f }; // x=u, y=v, z - длина прицела

  texture_coord texYp { 0.0f, 0.0f };
  texture_coord texYn {};
  texture_coord texXp {};
  texture_coord texXn {};
  texture_coord texZp {};
  texture_coord texZn {};

  char set_mouse_ptr = 0;           // запрос смены типа курсора {-1, 0, 1}
  void resize(u_int w, u_int h);
};
extern main_window WinParams;


class db
{
  public:
    db(void) {}
   ~db(void) {}
    v_str open_map(const std::string &); // загрузка данных карты
    v_str open_app(const std::string &); // загрузка данных приложения
    void map_name_save(const std::string &Dir, const std::string &MapName);
    v_ch map_name_read(const std::string & dbFile);
    void save(const camera_3d &Eye);
    void save(const main_window &WinParams);
    void init_map_config(const std::string &);
    void load_template(int level, const std::string &fname);   // загрузка шаблона из файла БД

  private:
    std::string MapDir       {}; // директория текущей карты (со слэшем в конце)
    std::string MapPFName    {}; // имя файла карты
    std::string CfgMapPFName {}; // файл конфигурации карты/вида
    std::string CfgAppPFName {}; // файл глобальных настроек приложения
    wsql SqlDb {};

    // == L-O-D 1 ==

    std::map<i3d, voxel> TplRigs_1 {};     // Шаблон карты - для создания новых элементов.
    int tpl_1_side = 16;                 // длина стороны шаблона
    void init_app_config(const std::string &);
    v_str load_config(size_t params_count, const std::string &file_name);
};

} //tr
#endif
