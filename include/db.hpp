/*
 * file: db.hpp
 *
 * Заголовочный файл класса управления базой данных
 *
 */

#ifndef DB_HPP
#define DB_HPP

#include "wsql.hpp"
#include "vox.hpp"
#include "framebuf.hpp"

namespace tr {

class wglfw;
class IUserInput
{
public:
    IUserInput(void) = default;
    virtual ~IUserInput(void) = default;

    virtual void mouse_event(int button, int action, int mods) = 0;
    virtual void keyboard_event(int key, int scancode, int action, int mods) = 0;
};

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
  std::unique_ptr<frame_buffer> RenderBuffer = nullptr;    // рендер-буфер окна
  img* pWinGui = nullptr;               // текстура GUI окна

  bool is_open = true;  // состояние окна
  float aspect = 1.0f;  // соотношение размеров окна
  double xpos = 0.0;    // позиция указателя относительно левой границы
  double ypos = 0.0;    // позиция указателя относительно верхней границы
  int fps = 500;        // частота кадров (для коррекции скорости движения)
  glm::vec3 Cursor = { 200.f, 200.f, .0f }; // x=u, y=v, z - длина прицела

  void resize(u_int w, u_int h);
};

extern main_window AppWindow;

class ev_input: public IUserInput
{
  public:
    explicit ev_input(void) = default;

    // Запретить копирование и перенос экземпляров класса
    ev_input(const ev_input&) = delete;
    ev_input& operator=(const ev_input&) = delete;
    ev_input(ev_input&&) = delete;
    ev_input& operator=(ev_input&&) = delete;

    float dx = 0.f;    // смещение мыши в активном окне
    float dy = 0.f;
    int fb = 0;        // 3D движение front/back
    int rl = 0;        // -- right/left
    int ud = 0;        // -- up/down

    int on_front = 0; // нажата клавиша вперед
    int on_back  = 0; // нажата клавиша назад
    int on_right = 0; // нажата клавиша вправо
    int on_left  = 0; // нажата клавиша влево
    int on_up    = 0; // нажата клавиша вверх
    int on_down  = 0; // нажата клавиша вниз

    int scancode = -1;
    int mods = -1;
    int mouse = -1;
    int action = -1;
    int key = -1;
    std::string StringBuffer {};  // строка ввода пользователя
    bool text_mode = false;

    virtual void mouse_event(int _button, int _action, int _mods);
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods);
};

extern ev_input Input;


class db
{
  public:
    db(void) {}
   ~db(void) {}
    v_str open_app(const std::string &); // загрузка данных приложения
    v_str map_open(const std::string &); // загрузка данных карты
    void map_close(const camera_3d &Eye);
    void map_name_save(const std::string &Dir, const std::string &MapName);
    v_ch map_name_read(const std::string & dbFile);
    void save_window_params(const main_window &AppWindow);
    void save_vox(vox*);
    void erase_vox(vox*);
    void init_map_config(const std::string &);
    std::unique_ptr<vox> get_vox(const i3d&, int);

  private:
    std::string MapDir       {}; // директория текущей карты (со слэшем в конце)
    std::string MapPFName    {}; // имя файла карты
    std::string CfgMapPFName {}; // файл конфигурации карты/вида
    std::string CfgAppPFName {}; // файл глобальных настроек приложения
    wsql SqlDb {};

    void init_app_config(const std::string &);
    v_str load_config(size_t params_count, const std::string &file_name);
};

} //tr
#endif
