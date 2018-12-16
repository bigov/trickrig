//=============================================================================
//
// file: rigs.hpp
//
// Элементы формирования пространства
//
//=============================================================================
#ifndef RDB_HPP
#define RDB_HPP

#include "main.hpp"
#include "glsl.hpp"
#include "snip.hpp"
#include "objl.hpp"
#include "wsql.hpp"

namespace tr
{
  // структура для обращения в тексте программы к индексам данных рига
  enum RIG_SHIFT_ID {
    SHIFT_X, SHIFT_Y, SHIFT_Z,        // координаты 3D
    SHIFT_YAW, SHIFT_PIT, SHIFT_ROLL, // рыскание (yaw), тангаж (pitch), крен (roll)
    SHIFT_ZOOM,                       // масштабирование
    SHIFT_DIGITS                      // число элементов
  };

  class rig // группа элементов, образующих объект пространства
  {
    public:
      int born = 0;                   // метка времени создания
      tr::i3d Origin {};              // координаты опорной точки
      float shift[SHIFT_DIGITS] = {
        0.f, 0.f, 0.f, // сдвиг поверхности относительно опорной точки
        0.f, 0.f, 0.f, // поворот по трем осям
        1.0f           // размер (масштабирование)
      };
      std::forward_list<tr::snip> Trick {}; // список поверхностей (снипов)
      bool in_vbo = false;                  // данные помещены в VBO
      // -------------------------------------
      rig(void): born(tr::get_msec()) {}    // конструктор по-умолчанию
      rig(const tr::rig &);                 // дублирующий конструктор
      rig& operator= (const tr::rig &);     // копирующее присваивание
  };

  //## Управление кэшем ригов с поверхности одного уровня LOD
  class rdb
  {
    private:
      std::map<tr::i3d, tr::rig> RigsDb {};  // карта поверхности
      std::map<tr::i3d, tr::rig> TplRigs {}; // шаблон карты 16х16
      int yMin = -100;
      int yMax = 100;
      int lod = 1; // размер стороны элементов в текущем LOD

      // Кэш адресов блоков данных в VBO, вышедших за границу рендера
      std::forward_list<GLsizeiptr> CachedOffset {};

      // Карта размещения cнипов по адресам в VBO
      std::unordered_map<GLsizeiptr, tr::snip*> VisibleSnips {};

      tr::vbo VBOdata = {GL_ARRAY_BUFFER};  // VBO вершин поверхности
      tr::glsl Prog3d {};                   // GLSL программа шейдеров

      GLuint space_vao = 0;                 // ID VAO
      GLsizei render_points = 0;            // число точек передаваемых в рендер

      void _load_16x16_obj(void);

    public:
      rdb(void);                              // конструктор
      void put_in_vbo(int, int, int);         // разместить данные в VBO буфере
      void put_in_vbo(tr::rig*, const f3d&);  // разместить данные в VBO буфере
      void remove_from_vbo(int, int, int);    // убрать трик из рендера
      void draw(void);                        // Рендер кадра
      void clear_cashed_snips(void);          // очистка промежуточного кэша

      bool save(const tr::i3d &, const tr::i3d &);
      void init(int, glm::vec3 = {0,0,0});       // загрузка уровня
      void highlight(const tr::i3d &);

      tr::rig* get(const glm::vec3 &);
      tr::rig* get(int x, int y, int z);
      tr::rig* get(const tr::i3d &);

      tr::i3d search_down(int, int, int);
      tr::i3d search_down(double, double, double);
      tr::i3d search_down(float, float, float);
      tr::i3d search_down(const glm::vec3 &);
      tr::rig load_tpl_rig(int, int, int);
  };

} //namespace tr

#endif
