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
#include "glsl.hpp"

namespace tr {

// Параметры и режимы окна приложения
class win_data: public IWindowInput
{
  public:
    explicit win_data(void) = default;

    // Запретить копирование и перенос экземпляров класса
    win_data(const win_data&) = delete;
    win_data& operator=(const win_data&) = delete;
    win_data(win_data&&) = delete;
    win_data& operator=(win_data&&) = delete;

    layout Layout {400, 400, 0, 0};      // размеры и положение
    u_int btn_w = 120;                   // ширина кнопки GUI
    u_int btn_h = 36;                    // высота кнопки GUI
    u_int minwidth = (btn_w + 16) * 4;   // минимально допустимая ширина окна
    u_int minheight = btn_h * 4 + 8;     // минимально допустимая высота окна
    img* pWinGui = nullptr;              // текстура GUI окна

    bool is_open = true;  // состояние окна
    float aspect = 1.0f;  // соотношение размеров окна
    int fps = 500;        // частота кадров (для коррекции скорости движения)

    glm::vec3 Sight = { 200.f, 200.f, .0f }; // положение и размер прицела
    double xpos = 0.0;             // позиция указателя относительно левой границы
    double ypos = 0.0;             // позиция указателя относительно верхней границы
    bool cursor_is_visible = true; // режим указателя мыши в окне

    float dx = 0.f;   // смещение мыши в активном окне между кадрами
    float dy = 0.f;

    int fb = 0;       // 3D движение front/back
    int rl = 0;       // -- right/left
    int ud = 0;       // -- up/down

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

    virtual void mouse_event(int _button, int _action, int _mods);
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods);
    virtual void reposition_event(int left, int top);
    virtual void resize_event(GLsizei width, GLsizei height);
    virtual void cursor_position_event(double x, double y);
    virtual void sight_position_event(double x, double y);
    virtual void close_event(void);
    virtual void cursor_hide(void);
    virtual void cursor_show(void);

    void layout_set(const layout &L);

};

extern win_data AppWindow;


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
    void save_window_layout(const layout&);
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
