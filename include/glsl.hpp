//============================================================================
//
// file: glsl.hpp
//
// GLSL shaders program class header
//
//============================================================================
#ifndef GLSL_HPP
#define GLSL_HPP

//#include "main.hpp"
#include "io.hpp"

namespace tr
{
  class glsl
  {
    public:
      glsl(void);
      ~glsl(void);

      void init(void);
      void destroy(void);

      // Запретить копирование и перенос экземпляров класса
      glsl(const glsl&) = delete;
      glsl& operator=(const glsl&) = delete;
      glsl(glsl&&) = delete;
      glsl& operator=(glsl&&) = delete;

      void attach_shader(GLenum shader_type, 
        const std::string& file_name);

      void attach_shaders(
        const std::string& vertex_shader_filename,
        const std::string& fragment_shader_filename);

      //void attach_shaders(
      //  const std::string& vertex_shader_filename,
      //  const std::string& geometric_shader_filename,
      //  const std::string& fragment_shader_filename);

      GLuint get_id(void);
      std::map<std::string, GLuint> Atrib {};

      GLuint attrib_location_get(const char *);
      GLint uniform_location_get(const char *);
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

  };

extern glsl Prog3d;
extern glsl screenShaderProgram;

} //namespace tr
#endif
