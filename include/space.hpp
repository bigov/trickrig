/**
 *
 * file: space.hpp
 *
 * Заголовок класса управления виртуальным пространством
 *
 */

#ifndef SPACE_HPP
#define SPACE_HPP

#include "area.hpp"
#include "framebuf.hpp"

using sys_clock = std::chrono::system_clock;

namespace tr
{
  class space: public interface_gl_context
  {
    public:
      space(wglfw* OpenGLContext);
      ~space(void);

      void enable(void);
      bool render(void);
      bool is_ready(void) const {return ready;}

      virtual void resize_event(int width, int height);
      virtual void cursor_event(double x, double y);
      virtual void mouse_event(int _button, int _action, int _mods);
      virtual void keyboard_event(int _key, int _scancode, int _action, int _mods);
      virtual void focus_lost_event();

      camera_3d Eye {};  // главная камера 3D вида
      int FPS = 500;     // частота кадров (для коррекции скорости движения)

    private:
      space(const space &);
      space operator=(const space &);

      wglfw* OglContext = nullptr;

      img ImHUD { 0, 0 };      // Текстура HUD окна приложения
      GLuint texture_hud = 0;  // ID HUD текстуры в GPU

      px bg_hud  {0x00, 0x88, 0x00, 0x40}; // Фон панели HUD

      bool ready = false;
      bool focus_is_on = false;
      float dx = 0.f;    // Cмещение мыши в активном окне между кадрами
      float dy = 0.f;    // в режиме 3D (режим прицела) при скрытом курсоре.
      double xpos = 0.0; // позиция указателя относительно левой границы
      double ypos = 0.0; // позиция указателя относительно верхней границы

      int fb_way = 0;    // 3D движение front/back
      int rl_way = 0;    // -- right/left
      int ud_way = 0;    // -- up/down

      int on_front = 0;  // нажата клавиша вперед
      int on_back  = 0;  // нажата клавиша назад
      int on_right = 0;  // нажата клавиша вправо
      int on_left  = 0;  // нажата клавиша влево
      int on_up    = 0;  // нажата клавиша вверх
      int on_down  = 0;  // нажата клавиша вниз

      int mouse = -1;
      int key = -1;
      int scancode = -1;
      int action = -1;
      int mods = -1;

      const float
        hPi = static_cast<float>(acos(0)), // половина константы "Пи" (90 градусов)
        Pi  = 2 * hPi,                     // константа "Пи"
        dPi = 2 * Pi;                      // двойная "Пи
      const float up_max = hPi - 0.001f;   // Максимальный угол вверх
      const float down_max = -up_max;      // Максимальный угол вниз

      std::unique_ptr<frame_buffer> RenderBuffer = nullptr; // рендер-буфер окна
      double cycle_time;            // время (в секундах) на рендер кадра

      glm::vec3 light_direction {}; // направление освещения
      glm::vec3 light_bright {};    // яркость света
      // Индексы вершин подсвечиваемого вокселя, на который направлен курсор (центр экрана)
      u_int id_point_0 = 0;         // индекс начальной вершины
      u_int id_point_8 = 0;         // индекс последней вершины

      // GPU control
      GLuint texture_id = 0;

      // LOD control
      std::unique_ptr<area> Area4 = nullptr; // Управление пространством вокселей
      const int size_v4 = 32;                // размер стороны вокселя
      const int border_dist_b4 = 24;         // число элементов от камеры до отображаемой границы

      // Camera control
      float rl=0.f, ud=0.f, fb=0.f; // скорость движения по направлениям

      // TODO: измерять средний за 10 сек. fps, и пропорционально менять скорость перемещения
      float speed_rotate = 0.001f; // скорость поворота (радиан в секунду) камеры
      float speed_moving = 10.f;   // скорость перемещения (в секунду) камеры

      float vision_angle = 50.f;   // угол зрения для расчета матрицы проекции
      float fovy = (hPi/90.f)*vision_angle;
      float zNear = 1.f;          // расстояние до ближней плоскости матрицы проекции
      float zFar  = 10000.f;      // расстояние до дальней плоскости матрицы проекции
      glm::mat4 MatProjection {}; // матрица проекции 3D сцены
      glm::mat4 MatMVP  {};       // Матрица преобразования
      glm::mat4 MatView {};       // матрица вида
      glm::vec3
        UpWard {0.0, -1.0, 0.0},  // направление наверх
        ViewTo {};                // направление взгляда

      void calc_render_time(void);
      void load_textures(void);
      void calc_position();
      bool check_keys();

      void hud_load(void);
      void hud_draw(void);
  };

} //namespace
#endif
