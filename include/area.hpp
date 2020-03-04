/**
 *
 * file: area.hpp
 *
 * Заголовок класса управления элементами области 3D пространства
 *
 */

#ifndef AREA_HPP
#define AREA_HPP

//#include "vox.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
#include "config.hpp"

namespace tr
{

extern void db_control( std::shared_ptr<trgl> OpenGLContext,
                        std::shared_ptr<glm::vec3> CameraLocation,
                        GLuint id, GLsizeiptr size );
struct vbo_map
{
  int x, y, z;
  uchar face_id;
};

enum COORDXZ { XL, ZL, sizeL};

///
/// \brief class area
/// \details Управление картой воксов
class area
{
  public:
    area(void) = delete;
    area(GLuint vbo_id, GLsizeiptr vbo_size);
    ~area(void) {}

    // Запретить копирование и перенос экземпляров класса
    area(const area&) = delete;
    area& operator=(const area&) = delete;
    area(area&&) = delete;
    area& operator=(area&&) = delete;

    void init(std::shared_ptr<glm::vec3> CameraLocation);
    bool recalc_borders(void);

  private:
    std::shared_ptr<glm::vec3> ViewFrom = nullptr;
    std::unique_ptr<vbo_ctrl> VboCtrl = nullptr;   // Управление содержимым VBO

    // В контрольный массив (VboMap) записываются координаты Origin воксов для видимых сторон,
    // переданных в VBO в порядке их размещения. Соответственно, размер массива должен
    // соотвествовать зарезервированному размеру VBO. Адрес блока данных стороны в VBO
    // определяется умножением значения индекса элемента в VboMap на размер блока.
    std::unique_ptr<vbo_map[]> VboMap = nullptr; // Контрольный массив

    int side_len     = 0;             // Длина стороны вокса
    float f_side_len = 0.f;           // Длина стороны вокса
    int lod_dist = 0;                 // Расстояние от камеры до границы LOD, кратное vox_side_len

    float last[2]      = {0.f, 0.f};  // последнее считанное положение камеры
    float curr[2]      = {0.f, 0.f};  // текущее положение камеры
    float move_dist[2] = {0.f, 0.f};  // расстояние, пройденное между запросами
    int origin[2]      = {0, 0};      // Origin вокса, над которым камера

    void load(int x, int z);
    void vox_append(const int x, const int y, const int z, const uchar face_id);
    void vox_remove(const int x, const int y, const int z);
    bool change_control(void);
    void redraw_borders_x(int, int);
    void redraw_borders_z(int, int);
    void truncate(int x, int z);
};



} //namespace tr

#endif
