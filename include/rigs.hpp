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

  class rig //## свойства элемента пространства
  {
    public:
      int born = 0;                         // метка времени создания
      float shift[SHIFT_DIGITS] = {
        0.f, 0.f, 0.f, // сдвиг поверхности относительно опорной точки
        0.f, 0.f, 0.f, // поворот по трем осям
        1.0f           // размер (масштабирование)
      };
      std::forward_list<tr::snip> Trick {}; // список поверхностей
      bool in_vbo = false;                  // данные помещены в VBO
      // -------------------------------------
      rig(void): born(tr::get_msec()) {}    // конструктор по-умолчанию
      rig(const tr::rig &);                 // дублирующий конструктор
      rig& operator= (const tr::rig &);     // копирующее присваивание
  };

  //## Клас для организации доступа к элементам уровня LOD
  class rigs
  {
    private:
      std::map<tr::i3d, tr::rig> Db {};
      int yMin = -100;
      int yMax = 100;
      int lod = 1.0f; // размер стороны элементов в текущем LOD

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

      void set_visible(int, int, int); // разместить трик в графическом буфере
      void set_hiding(int, int, int);  // убрать трик из рендера
      void draw(const glm::mat4 &); // Рендер кадра
      void clear_cashed_snips(void);

      bool save(const tr::i3d &, const tr::i3d &);
      void init(int);     // загрузка уровня
      rig* get(int x, int y, int z);
      rig* get(const tr::i3d&);
      i3d search_down(int x, int y, int z);
      i3d search_down(const glm::vec3 &);
      bool exist(int, int, int);
  };

} //namespace tr

#endif
