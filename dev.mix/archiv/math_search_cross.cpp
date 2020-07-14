/*
 * поиск пересечения "линии взгляда" с поверхностью при помощи
 * математических вычислений.
 *
 */


/// Вычисление расположения отрезка (a, b) относительно (слева или справа)
/// точки c нулевыми координатами на плоскости (x,y)
///
inline bool dir(const glm::vec4 &a, const glm::vec4 &b)
{
  return ((0.0f - b.x) * (a.y - b.y) - (a.x - b.x) * (0.0f - b.y)) > 0.0f;
}


/// \brief Проверка размещения четырехугольника в центре плоскости камеры
/// \return
///
/// Выбор вершин производится в направлении против часовой стрелки
///
inline bool is_target(glm::vec4 &a, glm::vec4 &b, glm::vec4 &c, glm::vec4 &d)
{
  return dir(a, b) && dir(b, c) && dir(c, d) && dir(d, a);
}


//
// Расчет координат рига, на который направлен взгляд. Расчет производится 
// при помощи матрицы вида. В приведенном алгоритме проверяется отлько верхняя
// сторона куба (верхний снип рига).
//
void calc_selected_area(glm::vec3 & LookDir)
{
  Selected = { 0, 0, 0 };                        // координаты выбранного рига
  glm::vec3 step = glm::normalize(LookDir)/1.2f; // длина шага поиска
  int i_max = 4;                                 // количество шагов проверки

  glm::vec3 check = Eye.ViewFrom;                // переменная поиска
  tr::rig* R = nullptr;
  for(int i = 0; i < i_max; ++i)
  {
    R = RigsDb0.get(check);
    if(nullptr != R) break;
    check += step;
  }
  if(nullptr == R) return;                       // если пересечение не найдено - выход

  // Проверяем верхний снип в найденном риге
  auto S = R->SideYp.front();
  glm::vec4 mv { R->Origin.x, R->Origin.y, R->Origin.z, 0 };

  // Проекции точек снипа на плоскость камеры
  glm::vec4 A = MatView * (S.vertex_coord(0) + mv);
  glm::vec4 B = MatView * (S.vertex_coord(1) + mv);
  glm::vec4 C = MatView * (S.vertex_coord(2) + mv);
  glm::vec4 D = MatView * (S.vertex_coord(3) + mv);

  if(is_target(A, B, C, D))
  {
    Selected = R->Origin;
  }
  else
  { // Если луч взгляда не пересекает найденый снип, то берем следущий
    // по направлению взгляда:
    check += step;
    R = RigsDb0.get(check);
    if(nullptr != R) Selected = R->Origin;
  }
}


