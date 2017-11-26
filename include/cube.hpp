//============================================================================
//
// file: cube.hpp
//
// Геометрические фигуры
//
//============================================================================
#ifndef __CUBE_HPP__
#define __CUBE_HPP__

#include "main.hpp"
#include "io.hpp"

namespace tr
{

  class Cube
  {
    public:
      const int // размер одного атрибута на каждую вершину (в байтах)
        c_size = sizeof(GLfloat) * 3, // 3D координаты
        n_size = sizeof(GLfloat) * 3, // координаты нормали
        t_size = sizeof(GLubyte) * 2, // UV координаты текстуры
        v_size = sizeof(GLfloat) * 8, // все атрибуты одним массивом
        i_size = sizeof(GLuint);      // индексы

    private:

      float side_lenght; // длина стороны куба

      const unsigned char sides_mask; // схема отображения сторон

      // Количество числовых значений (float) для описания одной вершины
      const unsigned vert_digits  = 8; // x, y, z, nx, ny, nz, U, V
      const unsigned cube_sides   = 6; // сторон в кубе
      const unsigned side_vertices = 4; // вершин на одну сторону
      const GLsizei side_indices = 6;  // индексов на сторону
      const GLsizeiptr // число байт для хранения данных одной грани
        side_i_bytes = i_size * 6, // индексов вершин грани
        side_v_bytes = v_size * 4, // всех атрибутов грани общим массивом
        side_c_bytes = c_size * 4, // 3D координат грани
        side_n_bytes = n_size * 4, // координат нормали грани
        side_t_bytes = t_size * 4; // UV координат текстуры грани

      GLfloat unit[ 8 * 24 ]; // массив данных вершин
      unsigned f_unit = 0; // указатель положения в массиве unit[]

      GLuint index[36]; // массив индексов вершин куба (6*6=36)
      unsigned f_index = 0;  // указатель положения в массиве индексов

      GLfloat coord3d[3 * 24]; unsigned f_coord3d = 0;
      GLfloat normals[3 * 24]; unsigned f_normals = 0;
      GLfloat texture[2 * 24]; unsigned f_texture = 0;

      GLuint side_order[6] = {0, 1, 2, 2, 3, 0}; // порядок обхода вершин

    public:
      explicit Cube(GLfloat x, GLfloat y, GLfloat z, int type,
        float lengh, unsigned char sides_mask, GLuint start_index);

      GLfloat* v_data = &unit[0];  // адрес массива всех атрибутов вершин
      GLuint * i_data = &index[0]; // адрес массива индексов обхода
      GLfloat* c_data = &coord3d[0];
      GLfloat* n_data = &normals[0];
      GLfloat* t_data = &texture[0];

      GLsizeiptr
        i_bytes = 0, // размер массива индексов
        v_bytes = 0, // число байт в массиве (размер в байтах)
        c_bytes = 0,
        n_bytes = 0,
        t_bytes = 0;

      GLsizei num_indices = 0; //количество индексов
      GLuint num_vertices = 0; //количество активных точек

      void position_set(GLfloat, GLfloat, GLfloat);
      void texture_set(int);
      void setup(GLfloat, GLfloat, GLfloat, int, GLuint);

    private:
      Cube(const Cube &); // Дублирующий конструктор не требуется
      Cube & operator=(const Cube &); // копирующий конструктор не требуется

      glm::vec3 location {}; // координаты центра трехмерной фигуры

      int texture_id = 1;  // индекс текстуры по-умолчанию
      GLfloat texture_map[48]; // массив для хранения текстурной карты

      // Назначение для сторон куба индексов (частей) текстурной карты
      void texture_bind(int pX, int nX, int pY, int nY, int pZ, int nZ);
      // выбор формы текстурной карты по индексу (декдирование индекса)
      void texture_select(int);
      // установка текстуры на указанной стороне куба
      void texture_array_fill(GLfloat * surface, GLfloat u, GLfloat v);
      void unit_update(void);
      void unit_clear(void);
      void insert_vertex(float,float,float,float,float,float,float,float);
      void place_side(void);
  };
} //namespace tr

#endif //__CUBE_HPP__
