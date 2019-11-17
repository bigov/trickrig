#include "voxbuf.hpp"
namespace tr {


///
/// \brief cross
/// \param s
/// \return номер стороны, противоположной указанной в параметре
///
u_char side_opposite(u_char s)
{
  switch (s)
  {
    case SIDE_XP: return SIDE_XN;
    case SIDE_XN: return SIDE_XP;
    case SIDE_YP: return SIDE_YN;
    case SIDE_YN: return SIDE_YP;
    case SIDE_ZP: return SIDE_ZN;
    case SIDE_ZN: return SIDE_ZP;
    default: return UCHAR_MAX;
  }
}


///
/// \brief vox_buffer::vox_buffer
/// \param side_len    - длина стороны вокса
/// \param VBO_pointer - указатель на буфер VBO
/// \param P0          - 3D точка начала области
/// \param P1          - 3D точка конца области
/// \details Запрос из базы данных воксов и передача их в рендер
///
vox_buffer::vox_buffer(int side_len, int b_dist, const i3d P0, const i3d P1)
{
  vox_side_len = side_len;
  border_dist = b_dist;

  pVBO = std::make_unique<vbo_ext> (GL_ARRAY_BUFFER);
  init_vao();

  i3d vP {};
  for(vP.x = P0.x; vP.x<= P1.x; vP.x += vox_side_len)
  for(vP.y = P0.y; vP.y<= P1.y; vP.y += vox_side_len)
  for(vP.z = P0.z; vP.z<= P1.z; vP.z += vox_side_len) vox_load(vP);
}


///
/// \brief vox_buffer::init_vao
///
void vox_buffer::init_vao(void)
{
  pVBO->clear();

  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  // Число элементов в кубе с длиной стороны LOD (2*dist_xx) элементов:
  u_int n = static_cast<u_int>(pow((border_dist + border_dist + 1), 3));

  // Размер данных VBO для размещения сторон вокселей:
  pVBO->allocate(n * bytes_per_side);

  // настройка положения атрибутов
  pVBO->attrib(Prog3d.Atrib["position"],
    3, GL_FLOAT, GL_FALSE, bytes_per_vertex, 0 * sizeof(GLfloat));

  pVBO->attrib(Prog3d.Atrib["color"],
    4, GL_FLOAT, GL_TRUE, bytes_per_vertex, 3 * sizeof(GLfloat));

  pVBO->attrib(Prog3d.Atrib["normal"],
    3, GL_FLOAT, GL_TRUE, bytes_per_vertex, 7 * sizeof(GLfloat));

  pVBO->attrib(Prog3d.Atrib["fragment"],
    2, GL_FLOAT, GL_TRUE, bytes_per_vertex, 10 * sizeof(GLfloat));

  //
  // Так как все четырехугольники сторон индексируются одинаково, то индексный массив
  // заполняем один раз "под завязку" и забываем про него. Число используемых индексов
  // будет всегда соответствовать числу элементов, передаваемых в процедру "glDraw..."
  //
  size_t idx_size = static_cast<size_t>(6 * n * sizeof(GLuint)); // Размер индексного массива
  GLuint *idx_data = new GLuint[idx_size];                       // данные для заполнения
  GLuint idx[6] = {0, 1, 2, 2, 3, 0};                            // шаблон четырехугольника
  GLuint stride = 0;                                             // число описаных вершин
  for(size_t i = 0; i < idx_size; i += 6) {                      // заполнить массив для VBO
    for(size_t x = 0; x < 6; x++) idx_data[x + i] = idx[x] + stride;
    stride += 4;                                                 // по 4 вершины на сторону
  }
  vbo_base VBOindex = { GL_ELEMENT_ARRAY_BUFFER };               // индексный буфер
  VBOindex.allocate(static_cast<GLsizei>(idx_size), idx_data);   // и заполнить данными.
  delete[] idx_data;                                             // Удалить исходный массив.
  glBindVertexArray(0);
}


///
/// \brief vox_buffer::get_render_indices
/// \return
///
u_int vox_buffer::get_render_indices(void)
{
  return render_indices;
}


///
/// \brief vox_buffer::i3d_near
/// \param P
/// \param s
/// \param l
/// \return
///
///  Координаты опорной точки соседнего вокселя относительно указанной стороны
///
i3d vox_buffer::i3d_near(const i3d& P, u_char side)
{
  switch (side) {
    case SIDE_XP: return i3d{ P.x + vox_side_len, P.y, P.z };
    case SIDE_XN: return i3d{ P.x - vox_side_len, P.y, P.z };
    case SIDE_YP: return i3d{ P.x, P.y + vox_side_len, P.z };
    case SIDE_YN: return i3d{ P.x, P.y - vox_side_len, P.z };
    case SIDE_ZP: return i3d{ P.x, P.y, P.z + vox_side_len };
    case SIDE_ZN: return i3d{ P.x, P.y, P.z - vox_side_len };
    default:      return P;
  }
}


///
/// \brief vox_buffer::add_vox
/// \param P
/// \return vox*
///
/// \details В указанной 3D точке генерирует новый вокс c пересчетом
/// видимости своих сторон и окружающих воксов и вносит его в базу данных
///
vox* vox_buffer::add_vox_in_db(const i3d& P)
{
  auto pVox = cfg::DataBase.get_vox(P, vox_side_len);
  if (nullptr == pVox)
  {
    pVox = std::make_unique<vox> (P, vox_side_len);
    cfg::DataBase.save_vox(pVox.get());
  }
  data.push_back(std::move(pVox));
  return data.back().get();
}


///
/// \brief vox_buffer::append
/// \param i
///
/// \details Добавление вокса к указанной стороне
///
void vox_buffer::append(u_int id)
{
  if(id > (render_indices/indices_per_side) * bytes_per_side) return;

  GLsizeiptr offset = (id/vertices_per_side) * bytes_per_side;
  vox* pVox = vox_by_vbo(offset);
  if (nullptr == pVox) return;
  i3d P0 = i3d_near(pVox->Origin, pVox->side_id_by_offset(offset)); // координаты нового вокса

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы
    vox_wipe(vox_by_i3d(i3d_near(P0, side)));   // вокруг точки добавления вокса

  // Добавить вокс в БД и в рендер VBO
  vox* V = add_vox_in_db(P0);
  recalc_vox_visibility(V);
  vox_draw(V);

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
    vox_draw(vox_by_i3d(i3d_near(P0, side)));
}


///
/// \brief vox_buffer::remove
/// \param i - порядковый номер группы данных из буфера
///
/// \details Удаление вокса
///
void vox_buffer::remove(u_int i)
{
  if(i > (render_indices/indices_per_side) * bytes_per_side) return;
  GLsizeiptr offset = (i/vertices_per_side) * bytes_per_side; // по номеру группы - адрес смещения в VBO
  vox* V = vox_by_vbo(offset);
  if(nullptr == V) ERR("Error: try to remove unexisted vox");
  i3d P0 = V->Origin;

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы вокруг
    vox_wipe(vox_by_i3d(i3d_near(P0, side)));

  vox_wipe(V);                   // Выделенный вокс убрать из рендера
  cfg::DataBase.erase_vox(V);    // удалить из базы данных,
  vox_unload(P0);                // удалить из буфера
  recalc_around_visibility(P0);  // пересчитать видимость вокселей вокруг

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    vox_draw(vox_by_i3d(i3d_near(P0, side)));
}


///
/// \brief vox_buffer::vox_load
/// \param P0
/// \details Загрузить вокс из базы данных в буфер и в рендер
///
void vox_buffer::vox_load(const i3d& P0)
{
  auto pVox = cfg::DataBase.get_vox(P0, vox_side_len);

  if(nullptr != pVox)
  {
    data.push_back(std::move(pVox));
    vox* V = data.back().get();

    for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы
      vox_wipe(vox_by_i3d(i3d_near(P0, side)));   // вокруг точки добавления вокса

    recalc_vox_visibility(V); // пересчитать видимость сторон
    vox_draw(V);              // добавить в рендер

    for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
      vox_draw(vox_by_i3d(i3d_near(P0, side)));
  }
}


///
/// \brief area::vox_unload
/// \param P0
/// \details Выгрузить данные вокса из памяти, и убрать из рендера
///
void vox_buffer::vox_unload(const i3d& P0)
{
  auto it = std::find_if(data.begin(), data.end(),
                         [&P0](const auto& V){ return V->Origin == P0; });
  if(it == data.end()) return; // в этой точке вокса нет
  vox* V = it->get();
  if(V->in_vbo) vox_wipe(V);
  data.erase(it);
}


///
/// \brief vox_buffer::vox_draw
///
/// \details Добавление в графический буфер элементов
///
/// Координаты вершин хранятся в нормализованом виде, поэтому перед записью
/// в VBO данные копируются во временный кэш, где координаты пересчитываются
/// с учетом координат вокселя, после чего записываются в VBO. Адрес
/// размещения данных в VBO для каждой стороны вокселя сохраняется в параметрах вокселя.
///
void vox_buffer::vox_draw(vox* pVox)
{
  if(nullptr == pVox) return;
#ifndef NDEBUG
  if(pVox->in_vbo) ERR("vox_draw: duplicate draw");
#endif

  GLsizeiptr vbo_addr = 0;
  GLfloat buffer[digits_per_side];

  // каждая сторона вокса обрабатывается отдельно
  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->side_fill_data(side_id, buffer)) continue;
    vbo_addr = pVBO->append(buffer, bytes_per_side);
    pVox->offset_write(side_id, vbo_addr);     // запомнить адрес смещения стороны в VBO
    render_indices += indices_per_side;        // увеличить число точек рендера
  }
  pVox->in_vbo = true;
}


///
/// \brief rdb::remove_from_vbo
/// \param x
/// \param y
/// \param z
/// \details Убрать вокс из рендера
///
void vox_buffer::vox_wipe(vox* pVox)
{
  if(nullptr == pVox) return;
  if(!pVox->in_vbo) return;

  // Данные каждой из сторон перезаписываются данными воксов с хвоста VBO
  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->is_visible(side_id)) continue;      // если сторона невидима, то пропустить цикл
    GLsizeiptr dest = pVox->offset_read(side_id); // адрес освобождаемого блока данных
    pVox->offset_write(side_id, -1);              // чтобы функция поиска больше не нвходила этот вокс

    // free - новый адрес границы VBO (отсюда данные были перенесены по адресу "dest")
    GLsizeiptr free = pVBO->remove(dest, bytes_per_side); // переписать в VBO блок данных по адресу "dest"
    if(free == 0)
    { // VBO пустой
      render_indices = 0;
      pVox->in_vbo = false;
      return;
    }

    if (free != dest)  // на "dest" были перенесены данные c адреса "free".
    {
      vox* V = vox_by_vbo(free); // Найти вокс, чьи данные были перенесены
      if(nullptr == V) ERR("vox_wipe: find_in_buffer(" + std::to_string(free) + ")");
      V->offset_replace(free, dest); // скорректировать адрес размещения данных
    }
    // Если free == dest, то удаляемый блок данных был в конце VBO,
    // поэтому только уменьшаем число точек рендера
    render_indices -= indices_per_side;
  }
  pVox->in_vbo = false;
}


///
/// \brief vox_buffer::vox_by_i3d
/// \param P0
/// \return
/// \details Поиск в буфере вокса по его координатам
///
vox* vox_buffer::vox_by_i3d(const i3d& P0)
{
  auto it = std::find_if(data.begin(), data.end(),
                         [&P0](const auto& V){ return V->Origin == P0; });
  if (it == data.end()) return nullptr;
  else return it->get();
}


///
/// \brief vox_buffer::vox_by_vbo
/// \param addr
/// \return
/// \details Поиск в буфере вокса, у которого сторона размещена в VBO по указанному адресу
vox* vox_buffer::vox_by_vbo(GLsizeiptr addr)
{
  auto it = std::find_if( data.begin(), data.end(),
    [&addr](auto& V)
    {
      if(!V->in_vbo) return false;
      return V->side_id_by_offset(addr) < SIDES_COUNT;
    }
  );
  if (it == data.end()) return nullptr;
  else return it->get();
}

///
/// \brief vox_buffer::recalc_vox_visibility
/// \param R0
/// \details Пересчет видимости сторон вокса и его соседей вокруг него
///
void vox_buffer::recalc_vox_visibility(vox* pVox)
{
  if (nullptr == pVox) return;

  // С каждой стороны вокса проверить наличие соседа
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    vox* pNear = vox_by_i3d(i3d_near(pVox->Origin, side));
    if (nullptr != pNear)                      // Если есть соседний,
    {                                          // то соприкасающиеся
      pVox->visible_off(side);                 // стороны невидимы
      pNear->visible_off(side_opposite(side)); // у обоих воксов.
    } else
    {
      pVox->visible_on(side);                  // Иначе - сторона видимая.
    }
  }
}


///
/// \brief vox_buffer::recalc_around_visibility
/// \param P0
/// \details Пересчет видимости воксов вокруг опорной точки
///
void vox_buffer::recalc_around_visibility(i3d P0)
{
  vox* pVox = vox_by_i3d(P0);
  if (nullptr != pVox)
  {                              // Если в этой точке есть вокс,
    recalc_vox_visibility(pVox); // то вызвать пересчет видимости
    return;                      // его сторон и выйти.
  }

  // Если в указанной точке пусто, то у воксов вокруг нее
  // надо включить видимость прилегающих сторон
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    pVox = vox_by_i3d(i3d_near(P0, side));
    if (nullptr != pVox) pVox->visible_on(side_opposite(side));
  }
}



} //namespace tr
