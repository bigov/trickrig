//=============================================================================
//
// file: rigs.hpp
//
// Элементы формирования пространства
//
//=============================================================================
#ifndef __RIGS_HPP__
#define __RIGS_HPP__

#include "main.hpp"
#include "glsl.hpp"
#include "snip.hpp"
#include "objl.hpp"
#include "dbw.hpp"

namespace tr
{
  // структура для обращения в тексте программы к индексам данных рига
  enum RIG_SHIFT_ID {
    SHIFT_X, SHIFT_Y, SHIFT_Z,        // координаты 3D
    SHIFT_YAW, SHIFT_PIT, SHIFT_ROLL, // рыскание (yaw), тангаж (pitch), крен (roll)
    SHIFT_ZOOM,                       // масштабирование
    SHIFT_DIGITS                      // число элементов
  };

  class rig //##  элемент пространства
  {
  /* содержит:
   * - индекс типа элемента по которому выбирается текстура и поведение
   * - время установки (будет использоваться) для динамических блоков
   * - если данные записаны в VBO, то смещение адреса данных в GPU */

    public:
      // --------------------------------- конструкторы
      rig(void): born(tr::get_msec()) {} // пустой
      rig(const tr::rig &);              // дублирующий конструктор
      rig(const tr::f3d &);              // создающий снип в точке
      rig(int, int, int);                // создающий снип в точке
      rig(const tr::snip &);             // копирующий данные снипа

      int born;                            // метка времени создания

      /// Данные для размещения Ареа относительно опорной точки рига:
      /// - вектор смещения (x, y, z),
      /// - углы поворота (a, b, c),
      /// - масштаб отображения области
      float shift[SHIFT_DIGITS] =
        {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.0f};

      bool in_vbo = false;                 // данные расположены в VBO
      std::forward_list<tr::snip> Area {}; // список поверхностей

      rig& operator= (const tr::rig &); // копирующее присваивание
      void copy_snips(const tr::rig &); // копирование снипов с другого рига
      void add_snip(const tr::f3d &);   // добавление в риг дефолтного снипа
  };

  //## Клас для организации доступа к элементам уровня LOD
  class rigs
  {
    private:
      std::map<tr::f3d, tr::rig> Db {};
      float yMin = -100.f;
      float yMax = 100.f;
      float db_gage = 1.0f; // размер стороны элементов в текущем LOD

      // Кэш адресов блоков данных в VBO, вышедших за границу рендера
      std::forward_list<GLsizeiptr> CachedOffset {};

      // Карта размещения cнипов по адресам в VBO
      std::unordered_map<GLsizeiptr, tr::snip*> VisibleSnips {};

      tr::vbo VBOdata = {GL_ARRAY_BUFFER};   // VBO вершин поверхности
      tr::glsl Prog3d {};   // GLSL программа шейдеров

      GLuint space_vao = 0; // ID VAO
      GLsizei render_points = 0; // число точек передаваемых в рендер

      void _load_16x16_obj(void);

    public:
      rigs(void);                  // конструктор

      void set_visible(const tr::f3d &);  // разместить риг в графическом буфере
      void set_hiding(const tr::f3d &);  // убрать риг из рендера
      void draw(const glm::mat4 &); // Рендер кадра
      void clear_cashed_snips(void);

      bool save(const tr::f3d &, const tr::f3d &);
      void init(float);     // загрузка уровня
      rig* get(float x, float y, float z);
      rig* get(const tr::f3d&);
      f3d search_down(float x, float y, float z);
      f3d search_down(const glm::vec3 &);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
