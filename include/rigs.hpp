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
#include "objl.hpp"

namespace tr
{
  class rig //##  элемент пространства
  {
  /* содержит:
   * - индекс типа элемента по которому выбирается текстура и поведение
   * - время установки (будет использоваться) для динамических блоков
   * - если данные записаны в VBO, то смещение адреса данных в GPU */

    public:
      // ----------------------------- конструкторы
      rig(void): born(get_msec()) { }   // пустой
      rig(const tr::rig &);             // дублирующий конструктор
      rig(const tr::f3d &);             // создающий снип в точке
      rig(int, int, int);               // создающий снип в точке

      //short int type = 0;             // тип: текстура, поведение, физика и т.п
      int born;                         // метка времени рождения
      std::forward_list<tr::snip> Area {};
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

    public:

      rigs(void){}          // пустой конструктор
      void init(float);     // загрузка уровня

      rig* blank(float x, float y, float z);
      rig* get(float x, float y, float z);
      rig* get(const tr::f3d&);
      f3d search_down(float x, float y, float z);
      f3d search_down(const glm::vec3 &);
      size_t size(void) { return Db.size(); }
      bool is_empty(float x, float y, float z);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
