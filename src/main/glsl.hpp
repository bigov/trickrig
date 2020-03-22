//============================================================================
//
// file: glsl.hpp
//
// GLSL shaders program class header
//
//============================================================================
#ifndef GLSL_HPP
#define GLSL_HPP

#include "io.hpp"

namespace tr
{
  class glsl
  {
    public:
      glsl(const std::list<std::pair<GLenum, std::string>>& L);
      ~glsl(void);

      // Запретить копирование и перенос экземпляров класса
      glsl(const glsl&) = delete;
      glsl& operator=(const glsl&) = delete;
      glsl(glsl&&) = delete;
      glsl& operator=(glsl&&) = delete;

      // Список описаний атрибутов для VBO
      std::list<glsl_attributes> AtribsList {};

      GLuint get_id(void);
      GLuint attrib(const char *);
      GLint uniform(const char *);
      void set_uniform(const char*, const glm::mat4 &);
      void set_uniform(const char*, const glm::vec3 &);
      void set_uniform(const char*, const glm::vec4 &);
      void set_uniform(const char* name, GLint u);
      void set_uniform(const char* name, GLfloat u);
      void set_uniform1ui(const char* name, GLuint u);
      void link(void);
      void use(void);
      void unuse(void);
      void validate(void);

    private:
      GLuint id = 0;
      GLint isLinked = 0;
      std::vector<GLuint> Shaders {};
      void attach_shader(GLenum shader_type, const std::string& file_name);
  };

} //namespace tr
#endif
