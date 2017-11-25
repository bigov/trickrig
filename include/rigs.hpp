//=============================================================================
//
// file: rigs.hpp
//
// Элементы формирования пространства
//
//=============================================================================
#ifndef __RIGS_HPP__
#define __RIGS_HPP__

#include <iostream>
#include <vector>
#include "cube.hpp"
#include "vbo.hpp"
#include "glsl.hpp"
#include "main.hpp"

namespace tr
{
	//## Вычисляет число миллисекунд от начала суток.
	//
	// Функция используется (в планах) для контроля вида динамических блоков,
	// у которых тип элемента (переменная t) изменяется во времени.
	extern int get_msec(void);

	struct Instance
	{
		//для начала настроим только изменение origin
		float origin[3] = {0.0f}; // положение центральной точки

		//остальное пока отключено
		//float normal[3] = {0.0f}; // координаты нормали
	  //float gage      = 1.0f;    // масштаб элемента)
	  //int tex2d[2]    = {0, 0}; // смещение координат на текстурной карте
	};

	//##  элемент пространства
	//
	// содержит значения
	// - координат центральной точки, в которой расположен;
	// - углов поворота по трем осям
	// - масштаб/размер
	// - индекс типа элемента по которому выбирается текстура и поведение
	// - время установки (оспользуется для динамических блоков
	//
	struct Rig
	{
		float angle[3] = {0.f, 0.f, 0.f}; // угол поворота

		short int type = 0; // тип элемента (текстура, поведение, физика и т.п)
		unsigned char neighbors = TR_T00; // маска расположения соседних блоков.
																	    // Если значение == 64, то данных нет
		int time; // время создания
		std::list<GLsizeiptr> idx {}; // адреса атрибутов инстансов в VBO_Inst
		Rig(): time(get_msec()) {};
		Rig(short t): type(t), time(get_msec()) {};
		void idx_update(GLsizeiptr idSource, GLsizeiptr idTarget);
	};

	//## База данных элементов пространства
	//
	class Rigs
	{
		private:
			std::map<tr::f3d, tr::Rig> db{};
			bool emplace_complete = false;

		public:
			GLuint vert_count = 0; // сумма вершин всей сцены, переданных в буфер
			float gage = 1.f;		// размер/масштаб эементов в данном блоке

			Rigs(void){};
			Rig* get(const f3d&);
			Rig* get(float x, float y, float z);
			f3d search_down(float x, float y, float z);
			f3d search_down(const glm::vec3&);
			size_t size(void) { return db.size(); }
			void emplace(float x, float y, float z, short t);
			void stop_emplacing(void) { emplace_complete = true; };
			bool is_empty(float x, float y, float z);
			bool exist(float x, float y, float z);
			unsigned char sides_map(const f3d&);
			GLint edges_map(const f3d&, const unsigned char);
			void post_key(const f3d& c, GLsizeiptr i);
	};

} //namespace tr

#endif
