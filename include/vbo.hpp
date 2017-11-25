//============================================================================
//
// file: vbo.hpp
//
// Header of the GLSL VBOs control class
//
//============================================================================
#ifndef __VBO_HPP__
#define __VBO_HPP__

#include "main.hpp"
#include "config.hpp"

namespace tr {

class VBO {
	private:
		GLuint id = 0;            // индекс VBO
		GLsizeiptr allocated = 0; // резервировируемый размер буфера
		GLsizeiptr size  = 0;   // число байт, переданных в VBO буфер GPU

	public:
		VBO(void) {};
		void Allocate(GLsizeiptr allocated);
		void Allocate(GLsizeiptr allocated, const GLvoid* data);
		void Attrib(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
		void AttribI(GLuint, GLint, GLenum, GLsizei, const GLvoid*);
		void Reduce(GLintptr, GLintptr, GLsizeiptr);
		GLsizeiptr SubData(GLsizeiptr data_size, const GLvoid* data);
		void SubData(GLsizeiptr, const GLvoid*, GLsizeiptr);
		void Resize(GLsizeiptr size);
};

} //tr
#endif

