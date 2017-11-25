//============================================================================
//
// file: vbo.cpp
//
// Class GLSL VBOs control
//
//============================================================================
#include "vbo.hpp"

namespace tr {

//## Настройка границы контроля данных буфера
//
void VBO::Resize(GLsizeiptr new_size)
{
	if(new_size == size) return;
	if(new_size < 0) ERR("VBO: negative value of new size");
	if(new_size > allocated) ERR("VBO: overfolw value of new size");
	size = new_size;
	return;
}


//## Cоздание нового буфера указанного в параметре рамера
//		
void VBO::Allocate(GLsizeiptr al)
{
	if(0 != id) ERR("ERROR: trying to re-init VBO");
	allocated = al;
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, allocated, 0, GL_STATIC_DRAW);

	#ifndef NDEBUG //--контроль создания буфера--------------------------------
	GLint d_size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &d_size);
	assert(allocated == d_size);
	#endif //------------------------------------------------------------------
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	size = 0;
	return;
}

//## Cоздание и заполнение буфера с указанным в параметрах данными
//		
void VBO::Allocate(GLsizeiptr al, const GLvoid* data)
{
	if(0 != id) ERR("ERROR: trying to re-init VBO");
	allocated = al;
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, allocated, data, GL_STATIC_DRAW);

	#ifndef NDEBUG //--контроль создания буфера--------------------------------
	GLint d_size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &d_size);
	assert(allocated == d_size);
	#endif //------------------------------------------------------------------
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	size = al;
	return;
}

//## Настройка атрибутов для float
//
void VBO::Attrib(GLuint index, GLint d_size, GLenum type, GLboolean normalized,
	GLsizei stride, const GLvoid* pointer)
{
	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glVertexAttribPointer(index, d_size, type, normalized, stride, pointer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return;
}

//## Настройка атрибутов для int
//
void VBO::AttribI(GLuint index, GLint d_size, GLenum type,
	GLsizei stride, const GLvoid* pointer)
{
	glEnableVertexAttribArray(index);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glVertexAttribIPointer(index, d_size, type, stride, pointer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return;
}

//## Сжатие буфера атрибутов за счет перемещения данных из хвоста
// в неиспользуемый промежуток и уменьшение текущего индекса
//
void VBO::Reduce(GLintptr src, GLintptr dst, GLsizeiptr d_size)
{
	//return;
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glCopyBufferSubData(GL_ARRAY_BUFFER, GL_ARRAY_BUFFER, src, dst, d_size);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	size -= d_size;
	return;
}

//## Внесение данных с контролем границы максимального размера буфера
//
GLsizeiptr VBO::SubData(GLsizeiptr d_size, const GLvoid* data)
{
	#ifndef NDEBUG //--проверка свободного места в буфере----------------------
	if ((allocated - size) < d_size) ERR("FAILURE: VBO is overflow");
	#endif //------------------------------------------------------------------

	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferSubData(GL_ARRAY_BUFFER, size, d_size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLsizeiptr res = size;
	size += d_size;
	return res;
}

//## Внесение данных на место указанной группы атрибутов
// с контролем границы максимального размера буфера
//
void VBO::SubData(GLsizeiptr d_size, const GLvoid* data, GLsizeiptr idx)
{
	#ifndef NDEBUG //--проверка свободного места в буфере----------------------
	if (idx > (size - d_size)) ERR("VBO: bad index for update attrib group");
	if ((allocated - size) < d_size) ERR("FAILURE: VBO is overflow");
	#endif //------------------------------------------------------------------

	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferSubData(GL_ARRAY_BUFFER, idx, d_size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return;
}

} //tr
