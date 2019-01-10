/*
 *
 * file: vbo.hpp
 *
 * Header of the GLSL VBOs control class
 *
 */

#ifndef VBO_HPP
#define VBO_HPP

#include "main.hpp"
#include "rig.hpp"

namespace tr{

class vbo
{
  public:
    u_int render_points = 0;          // число точек передаваемых в рендер

    vbo(GLenum type): gl_buffer_type(type) {}
    ~vbo(void) {}

    void allocate(GLsizeiptr allocated);
    void allocate(GLsizeiptr allocated, const GLvoid* data);
    void attrib(GLuint, GLint, GLenum, GLboolean, GLsizei, size_t);
    void attrib_i(GLuint, GLint, GLenum, GLsizei, const GLvoid*);
    void jam_data(GLintptr, GLintptr, GLsizeiptr);
    void data_append_tmp(GLsizeiptr d_size, const GLvoid* data);
    void shrink(GLsizeiptr);
    GLsizeiptr get_hem(void) { return hem; }
    void clear_cashed_snips(void);            // очистка промежуточного кэша

    // Функции управления данными снипа в буферах VBO данных и VBO индекса
    void update(GLfloat* s_data, const f3d&, GLsizeiptr);
    void vbo_jam(snip*, GLintptr);

    void data_place(snip& Snip, const f3d&);   // разместить данные в VBO буфере
    void data_remove(std::vector<snip>&);      // убрать данные из рендера

private:
    GLuint id = 0;            // индекс VBO
    GLsizeiptr allocated = 0; // (максимальный) выделяемый размер буфера
    GLsizeiptr hem = 0;       // граница размещения данных в VBO
    GLenum gl_buffer_type;
    std::unordered_map<GLsizeiptr, snip*> VisibleSnips {}; // Карта размещения cнипов по адресам в VBO
    std::forward_list<GLsizeiptr> CachedOffset {}; // Кэш адресов блоков данных в VBO, вышедших за границу рендера

    void data_update(GLsizeiptr, const GLvoid*, GLsizeiptr offset);
    GLsizeiptr data_append(GLsizeiptr data_size, const GLvoid* data);
    GLsizeiptr append(GLfloat*, const f3d&);

};

} //tr
#endif

