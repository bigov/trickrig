/**
 *
 * file: area.hpp
 *
 * Заголовок класса управления элементами области 3D пространства
 *
 */

#ifndef AREA_HPP
#define AREA_HPP

#include <queue>
//#include "voxesdb.hpp"
#include "vox.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
#include "config.hpp"

namespace tr
{

///
/// \brief class area
/// \details Управление картой воксов
class area
{
  public:
    area(void) {}
    ~area(void) {}

    // Запретить копирование и перенос экземпляров класса
    area(const area&) = delete;
    area& operator=(const area&) = delete;
    area(area&&) = delete;
    area& operator=(area&&) = delete;

    void operator() (int length, int count, std::shared_ptr<glm::vec3> CameraLocation,
                     GLFWwindow* Context, GLenum buf_type, GLuint buf_id, GLsizeiptr buf_size);
    void init(int len, int elements, std::shared_ptr<glm::vec3> CameraLocation);
    bool recalc_borders(void);

  private:
    std::unique_ptr<vbo_ctrl> VBOctrl = nullptr;
    std::shared_ptr<glm::vec3> ViewFrom = nullptr;

    // В контрольный массив (GpuMap) записываются координаты Origin видимых сторон воксов,
    // переданных в VBO в порядке их размещения. Соответственно, размер массива должен
    // соотвествовать зарезервированному размеру VBO. Адрес размещения определяется
    // умножением значения индекса элемента в массиве на размер блока данных стороны.
    // Так как все видимые стороны вокса одновременно размещается GPU и одновременно
    // удаляются из нее, то нет необходимости различать каждую из сторон вокса - для каждой
    // в массив заносится одино и то-же значение координта Origin.
    struct vbo_map
    {
      int x, y, z;
      uchar side;
    };
    std::unique_ptr<vbo_map[]> VboMap = nullptr; // Контрольный массив

    float last[3]      = {0.f, 0.f, 0.f};  // последнее считанное положение камеры
    float curr[3]      = {0.f, 0.f, 0.f};  // текущее положение камеры
    float move_dist[3] = {0.f, 0.f, 0.f};  // расстояние, пройденное между запросами

    int side_len     = 0;     // Длина стороны вокса
    float f_side_len = 0.f;   // Длина стороны вокса
    int lod_dist = 0;         // Расстояние от камеры до границы LOD, кратное vox_side_len
    i3d Location {0, 0, 0};   // Origin вокса, над которым камера

    void redraw_borders_x(int, int);
    void redraw_borders_z(int, int);
    void load(int x, int z);
    void truncate(int x, int z);
};



} //namespace tr

#endif
