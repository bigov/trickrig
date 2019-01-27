//=============================================================================
//
// file: snip.hpp
//
// Элементы формирования пространства
//
//=============================================================================

#ifndef SNIP_HPP
#define SNIP_HPP

#include "main.hpp"

namespace tr
{

  //## Набор данных для формирования в GPU многоугольника с индексацией вершин
  struct snip
  {
    GLfloat u_size = 0.125f; // размер ячейки текстуры по U
    GLfloat v_size = 0.125f; // размер ячейки текстуры по V

    GLsizeiptr data_offset = 0; // положение блока данных в буфере GPU

    // Блок данных для передачи в буфер GPU
    // TODO: более универсальным тут было-бы (?) использование контейнера std::vector<GLfloat>,
    GLfloat data[tr::digits_per_snip] = {
   /*|---3D коодината--|---------RGBA цвет----|--вектор нормали--|--текстура--*/
      0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    snip(void) {}
    snip(const snip &);                 // дублирующий конструктор

    snip& operator=(const snip &);      // оператор присваивания
    void copy_data(const snip &);       // копирование данных из другого снипа
    void shift(const glm::vec3&);       // сдвиг снипа на вектор
    void flip_y(void);                  // переворот по вертикали
    void texture_set(GLfloat u, GLfloat v); // установка текстур
    void texture_fragment(GLfloat, GLfloat, const std::array<float, 8>&);
    glm::vec4 vertex_coord(size_t);
    glm::vec4 vertex_normal(size_t idx);
  };

} //namespace tr

#endif
