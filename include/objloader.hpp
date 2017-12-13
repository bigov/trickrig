#ifndef OBJLOADER_H
#define OBJLOADER_H
// подробное описание работы загрузчика тут:
// http://www.opengl-tutorial.org/ru/beginners-tutorials/tutorial-7-model-loading/

#include "main.hpp"
//#include <vector>
//#include <stdio.h>
//#include <string>
//#include <cstring>
//#include <glm/glm.hpp>

bool loadOBJ(const char * path,
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals);

bool loadAssImp(const char * path,
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals);

#endif
