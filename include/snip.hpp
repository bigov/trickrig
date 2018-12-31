//=============================================================================
//
// file: snip.hpp
//
// Элементы формирования пространства
//
//=============================================================================

#ifndef SNIP_HPP
#define SNIP_HPP

#include "vbo.hpp"

namespace tr
{
  // структура для обращения в тексте программы к индексам данных вершин по названиям
  enum SNIP_DATA_ID { X, Y, Z, W, R, G, B, A, NX, NY, NZ, NW, U, V, ROW_SIZE };

  //## Набор данных для формирования в GPU многоугольника с индексацией вершин
  struct snip
  {
    GLfloat u_size = 0.125f; // размер ячейки текстуры по U
    GLfloat v_size = 0.125f; // размер ячейки текстуры по V

    GLsizeiptr data_offset = 0; // положение блока данных в буфере GPU

    // Блок данных для передачи в буфер GPU
    // TODO: более универсальным тут было-бы (?) использование контейнера std::vector<GLfloat>,
    GLfloat data[tr::digits_per_snip] = {
   /*|-----3D коодината-----|-------RGBA цвет-------|-----вектор нормали----|---текстура---*/
      0.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   0.375f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.125f, 0.375f,
      1.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.125f, 0.500f,
      0.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,   0.500f,
    };

    snip(void) {}
    snip(const snip &);                 // дублирующий конструктор

    snip& operator=(const snip &);      // оператор присваивания
    void copy_data(const snip &);       // копирование данных из другого снипа
    void shift(const glm::vec3&);       // сдвиг снипа на вектор
    void flip_y(void);                  // переворот по вертикали

    void texture_set(GLfloat u, GLfloat v); // установка текстур
    void texture_fragment(GLfloat, GLfloat, const std::array<float, 8>&);

    // Функции управления данными снипа в буферах VBO данных и VBO индекса
    void vbo_append(const f3d&, vbo&);
    bool vbo_update(const f3d&, vbo&, GLsizeiptr);
    void vbo_jam (vbo&, GLintptr);
    glm::vec4 vertex_coord(size_t);
  };

} //namespace tr

#endif
