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
#include "snip.hpp"

namespace tr
{
  //##  элемент пространства
  class rig
  {
  /* содержит:
   * - индекс типа элемента по которому выбирается текстура и поведение
   * - время установки (будет использоваться) для динамических блоков
   * - если данные записаны в VBO, то смещение адреса данных в GPU */

    public:
      rig(const tr::f3d &);

      short int type = 0;        // тип элемента (текстура, поведение, физика и т.п)
      int time;                  // время создания
      std::forward_list<tr::snip> area {};

    //  void reload(const std::vector<std::pair<
    //    std::array<float, 3>,std::array<float, 3>>> &);

    private:
      rig(void) = delete;
      rig operator= (const tr::rig&) = delete;
      rig(const tr::rig&) = delete;
  };

  //## Клас для управления базой данных элементов пространства одного LOD.
  class rigs
  {
    private:
      std::map<tr::f3d, tr::rig> db {};

    public:
      GLuint vert_count = 0; // сумма вершин, переданных в VBO
      float gage = 1.f;      // размер стороны элементов в текущем LOD

      rigs(void){}
      rig* get(float x, float y, float z);
      f3d search_down(float x, float y, float z);
      f3d search_down(const glm::vec3 &);
      size_t size(void) { return db.size(); }
      void emplace(int x, int y, int z);
      bool is_empty(float x, float y, float z);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
