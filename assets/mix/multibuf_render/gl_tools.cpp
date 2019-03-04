#include "gl_tools.hpp"

static GLuint hdl_VAO = 0;
static GLuint pos_Buf = 0;
static GLuint col_Buf = 0;

///
/// \brief read_file
/// \param FNname
/// \return Указатель на массив считанных данных
/// \details Чтение содержимого текстового файла
///
std::unique_ptr<char[]> read_file(const std::string &FNname)
{
  // проверка наличия файла
  struct stat Buf;
  if (stat (FNname.c_str(), &Buf) != 0) ERR("Missing file: " + FNname);
  // чтение файла
  std::ifstream file(FNname, std::ios::in|std::ios::ate);
  file.exceptions(std::ios_base::badbit|std::ios_base::failbit);
  if (!file.is_open()) ERR("Can't open " + FNname);

  auto size = file.tellg();
  if(size < 1) return nullptr;
  auto data_size = static_cast<size_t>(size) + 1;
  auto data = std::make_unique<char[]>(data_size);

  file.seekg(0, std::ios::beg);
  file.read(data.get(), size);
  file.close();

  data[data_size - 1] = '\0';
  return data;
}


//## компиялция шейдера с контролем результата
void compile_shader(GLuint shader)
{
  glCompileShader(shader);
  GLint result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if(GL_FALSE == result)
  {
    GLint logLen;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    if(logLen > 0)
    {
      auto log_size = static_cast<unsigned>(logLen);
      auto log = std::make_unique<char[]>(log_size + 1);
      GLsizei written;
      glGetShaderInfoLog(shader, logLen, &written, log.get());
      log[log_size] = '\0';
      std::cout << "\n" << log.get() << '\n';
    }
    ERR("Error compiling SHADER\n");
  }
  return;
}


//### Компиляция и запуск GLSL программы
void gl_init_program(void)
{
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  if (!vertShader) ERR("Error create GL_VERTEX_SHADER");
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  if (!fragShader) ERR("Error create GL_FRAGMENT_SHADER");

  auto Buff = read_file("../vert.glsl");
  auto b = Buff.get();
  glShaderSource(vertShader, 1, &b, nullptr);
  compile_shader(vertShader);

  Buff = read_file("../frag.glsl");
  b = Buff.get();
  glShaderSource(fragShader, 1, &b, nullptr);
  compile_shader(fragShader);

  GLuint program = glCreateProgram();
  if (!program) ERR("Error creating GLSL program\n");
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (GL_FALSE == status)
  {
    GLint logLen;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen > 0)
    {
      auto log_size = static_cast<size_t>(logLen);
      auto log = std::make_unique<char[]>(log_size + 1);
      GLsizei written;
      glGetProgramInfoLog(program, logLen, &written, log.get());
      log[log_size] = '\0';
      std::cout << log.get() << '\n';
    }
    ERR("Failed to link GLSL program.\n");
  }
  glUseProgram(program);

  return;
}

//### Загрузка данных, установка начальных параметров
void gl_init_scene(void)
{
  glClearColor(0.5f, 0.69f, 1.0f, 1.0f);

  float pos_Data[] = {
    -0.8f, -0.8f, 0.0f,
     0.8f, -0.8f, 0.0f,
     0.0f,  0.8f, 0.0f,
  };

  float col_Data[] = {
    0.4f, 1.0f, 0.4f,
    1.0f, 0.4f, 0.4f,
    1.0f, 1.0f, 0.4f,
  };

  glGenVertexArrays(1, &hdl_VAO);
  glBindVertexArray(hdl_VAO);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glGenBuffers(1, &pos_Buf);
  glBindBuffer(GL_ARRAY_BUFFER, pos_Buf);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), pos_Data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glGenBuffers(1, &col_Buf);
  glBindBuffer(GL_ARRAY_BUFFER, col_Buf);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), col_Data, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindVertexArray(0);
  return;
}

void gl_draw_arrays(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(hdl_VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  return;
}

