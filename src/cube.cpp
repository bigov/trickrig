//============================================================================
//
// file: cube.cpp
//
// Составные части пространства
//
//    pY
//    |
//    |_____ pX
//    /
//   /pZ
//
//     6(-+-)~~~~~~~~2(++-)
//    /|            /|
//   / |           / |
//  7(-++)~~~~~~~~3(+++)
//  |  |          |  |
//  |  |          |  |
//  |  5(---)~~~~~|~~1(+--)
//  | /           | /
//  |/            |/
//  4(--+)~~~~~~~~0(+-+)
//
//============================================================================
#include <valarray>
#include "cube.hpp"

namespace tr
{
	// размерность текстурной карты (8х8)
	GLubyte texture_scale = 8;
	// шаг сетки текстуры для координат float
	float tS = 1.f/static_cast<float>(texture_scale);

//============================ Cube class ====================================

	////////
	// Заполнение массива данных
	//
	// 	Очередность сторон у куба выберем на основании:
	//
	//		CubeMapTarget[0] = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	//		CubeMapTarget[1] = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB;
	//		CubeMapTarget[2] = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB;
	//		CubeMapTarget[3] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB;
	//		CubeMapTarget[4] = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB;
	//		CubeMapTarget[5] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB;
	//
	// таким образом получаем:
	// 	------------------------------------
	// 	left, right, top, bottom, face, back
	//  ------------------------------------
	//
	void Cube::position_set(GLfloat x, GLfloat y, GLfloat z)
	{
		location = {x, y, z};
		unit_update();
		return;
	}

	//### Обнуление массива данных и связанных с ним чсетчиков
	void Cube::unit_clear(void)
	{
		f_coord3d = 0;
		f_normals = 0;
		f_texture = 0;
		f_unit = 0;

		unit[f_unit] = '\0';

		v_bytes = 0;
		i_bytes = 0;
		c_bytes = 0,
		n_bytes = 0,
		t_bytes = 0;

		num_vertices = 0;
		num_indices = 0;

		return;
	}

	//### добавление индекса для обхода стороны куба
	//
	void Cube::place_side(void)
	{
		// insert one side indices
		GLuint step = num_vertices + index[0];
		for(unsigned i = 0; i < 6; i++)
		{
			index[f_index] = side_order[i] + step;
			f_index++;
		}

		num_vertices += side_vertices;
		num_indices  += side_indices;
		v_bytes += side_v_bytes;
		i_bytes += side_i_bytes;
		c_bytes += side_c_bytes;
		n_bytes += side_n_bytes;
		t_bytes += side_t_bytes;

		return;
	}

	//### Ввод координат для одной вершины
	//
	void Cube::insert_vertex(float x, float y, float z,
		float nx, float ny, float nz, float u, float v)
	{
		/*
		// общий массив
		unit[f_unit++] = x;  // 3d координаты вершины
		unit[f_unit++] = y;
		unit[f_unit++] = z;
		unit[f_unit++] = nx; // 3d вектор нормали
		unit[f_unit++] = ny;
		unit[f_unit++] = nz;
		unit[f_unit++] = u;  // координаты текстуры
		unit[f_unit++] = v;
		*/

		coord3d[f_coord3d++] = x;
		coord3d[f_coord3d++] = y;
		coord3d[f_coord3d++] = z;

		normals[f_normals++] = nx;
		normals[f_normals++] = ny;
		normals[f_normals++] = nz;

		texture[f_texture++] = u;
		texture[f_texture++] = v;

		return;
	}

	//### Пересчет отображаемых граней куба в зависимости от наличия соседей
	//
	void Cube::unit_update(void)
	{
		GLfloat
			S = side_lenght / 2.f,
			x = location[0],
			y = location[1],
			z = location[2];
		GLfloat * t = texture_map;

		unit_clear();
		// +X
		if (sides_mask & TR_TpX)
		{
			place_side();
			insert_vertex(x+S, y-S, z+S, 1.f, 0.f, 0.f, t[0], t[1]);
			insert_vertex(x+S, y-S, z-S, 1.f, 0.f, 0.f, t[2], t[3]);
			insert_vertex(x+S, y+S, z-S, 1.f, 0.f, 0.f, t[4], t[5]);
			insert_vertex(x+S, y+S, z+S, 1.f, 0.f, 0.f, t[6], t[7]);
		}
		// -X
		if (sides_mask & TR_TnX)
		{
			place_side();
			insert_vertex(x-S, y-S, z-S, -1.f, 0.f, 0.f, t[8],  t[9]);
			insert_vertex(x-S, y-S, z+S, -1.f, 0.f, 0.f, t[10], t[11]);
			insert_vertex(x-S, y+S, z+S, -1.f, 0.f, 0.f, t[12], t[13]);
			insert_vertex(x-S, y+S, z-S, -1.f, 0.f, 0.f, t[14], t[15]);
		}
		// +Y
		if (sides_mask & TR_TpY)
		{
			place_side();
			insert_vertex(x-S, y+S, z+S, 0.f, 1.f, 0.f, t[16], t[17]);
			insert_vertex(x+S, y+S, z+S, 0.f, 1.f, 0.f, t[18], t[19]);
			insert_vertex(x+S, y+S, z-S, 0.f, 1.f, 0.f, t[20], t[21]);
			insert_vertex(x-S, y+S, z-S, 0.f, 1.f, 0.f, t[22], t[23]);
		}
		// -Y
		if (sides_mask & TR_TnY)
		{
			place_side();
			insert_vertex(x-S, y-S, z-S, 0.f, -1.f, 0.f, t[24], t[25]);
			insert_vertex(x+S, y-S, z-S, 0.f, -1.f, 0.f, t[26], t[27]);
			insert_vertex(x+S, y-S, z+S, 0.f, -1.f, 0.f, t[28], t[29]);
			insert_vertex(x-S, y-S, z+S, 0.f, -1.f, 0.f, t[30], t[31]);
		}
		// +Z
		if (sides_mask & TR_TpZ)
		{
			place_side();
			insert_vertex(x-S, y-S, z+S, 0.f, 0.f, 1.f, t[32], t[33]);
			insert_vertex(x+S, y-S, z+S, 0.f, 0.f, 1.f, t[34], t[35]);
			insert_vertex(x+S, y+S, z+S, 0.f, 0.f, 1.f, t[36], t[37]);
			insert_vertex(x-S, y+S, z+S, 0.f, 0.f, 1.f, t[38], t[39]);
		}
		// -Z
		if (sides_mask & TR_TnZ)
		{
			place_side();
			insert_vertex(x+S, y-S, z-S, 0.f, 0.f, -1.f, t[40], t[41]);
			insert_vertex(x-S, y-S, z-S, 0.f, 0.f, -1.f, t[42], t[43]);
			insert_vertex(x-S, y+S, z-S, 0.f, 0.f, -1.f, t[44], t[45]);
			insert_vertex(x+S, y+S, z-S, 0.f, 0.f, -1.f, t[46], t[47]);
		}
		return;
	}

	//## Начальная настройка объекта
	//
	void Cube::setup(GLfloat x, GLfloat y, GLfloat z, int t, GLuint start_idx)
	{
		f_index = 0;
		index[f_index] = start_idx;
		location = {x, y, z};
		texture_select(t);
		unit_update();
		return;
	}

	//## Конструктор
	//
	Cube::Cube(GLfloat x, GLfloat y, GLfloat z, int t, float l,
		unsigned char s, GLuint start_idx): side_lenght(l), sides_mask(s)
	{
		setup(x, y, z, t, start_idx);
		return;
	}

	////////
	// Заполнение массива координат текстуры для одной стороны куба
	//
	// Тут производится заполнение координат текстуры для трех последующих
	// точек по значению координат текстуры начальной точки.
	// Интервалы соответствуют константе шага текстурной сетки (tS)
	//
	void Cube::texture_array_fill(GLfloat * t, GLfloat x0, GLfloat y0)
	{
		GLfloat
			x1 = x0 + tS,
			y1 = y0 + tS;

		t[0] = x0; t[1] = y0;
		t[2] = x1; t[3] = y0;
		t[4] = x1; t[5] = y1;
		t[6] = x0; t[7] = y1;

		return;
	}

	////////
	// Заполнение массива текстурных координт для всех сторон куба
	// индивидуально при помощи указанной через индекс сетки текстурой
	//
	void Cube::texture_bind(int pX, int nX, int pY, int nY, int pZ, int nZ)
	{
		// pX (left side)
		texture_array_fill(&texture_map[0],
			static_cast<GLfloat>(pX % texture_scale) * tS,
			static_cast<GLfloat>(pX / texture_scale) * tS);

		//nX (right side)
		texture_array_fill(&texture_map[8],
			static_cast<GLfloat>(nX % texture_scale) * tS,
			static_cast<GLfloat>(nX / texture_scale) * tS);

		//pY (up side)
		texture_array_fill(&texture_map[16],
			static_cast<GLfloat>(pY % texture_scale) * tS,
			static_cast<GLfloat>(pY / texture_scale) * tS);

		//nY (down side)
		texture_array_fill(&texture_map[24],
			static_cast<GLfloat>(nY % texture_scale) * tS,
			static_cast<GLfloat>(nY / texture_scale) * tS);

		//pZ (front side)
		texture_array_fill(&texture_map[32],
			static_cast<GLfloat>(pZ % texture_scale) * tS,
			static_cast<GLfloat>(pZ / texture_scale) * tS);

		//nZ (back side)
		texture_array_fill(&texture_map[40],
			static_cast<GLfloat>(nZ % texture_scale) * tS,
			static_cast<GLfloat>(nZ / texture_scale) * tS);

		return;
	}

	////////
	//
	//
	void Cube::texture_set(int id)
	{
		texture_select(id);
		unit_update();
		return;
	}

	////////
	// Декодирование индекса в текстурную карту для куба
	//
	void Cube::texture_select(int t_id)
	{
		switch(t_id)
		{
			case 2:
				texture_bind(8, 8, 8, 8, 8, 8);
				break;
			case 9:
				texture_bind(1, 9, 17, 25, 33, 41);
				break;
			case 1:
			default:
				texture_bind(0, 0, 0, 0, 0, 0);
				texture_id = 1;
				return;
		}
		texture_id = t_id;
		return;
	}

} //namespace tr
