#include <iostream>
#include <cmath>

int main(int, char**)
{
  double
    Hpi = acos(0),
    pi  = 2 * Hpi,
    Dpi = 2 * pi;

  // положение камеры
  double P[] = { 0.0, 2.0, 0.0 };

/*
  double a = 4.1;
  double b = 1.3;
  //  |\
  // a| \ c
  //  |__\       Угол между "b" и "с" от -90 (-Hpi) до 90 (Hpi)
  //   b  (bc)

  double bc = atan(a/b);            // угол между b-c
  double c = fabs(b) /cos(bc);      // длина с
*/

  // точка на меше
  double V[] = { 0.0, 0.0, 2.0001 };

  double
    dX = P[0] - V[0],
    dY = P[1] - V[1],
    dZ = P[2] - V[2],
    v_a, v_t;
  if(0.0 == dX) dX -= 0.000001; // потому что
  if(0.0 == dY) dY -= 0.000001; // нельзя
  if(0.0 == dZ) dZ -= 0.000001; // делить на ноль

  v_a = atan(dZ / dX);

  if(dX > 0) v_a -= pi;

  if(v_a < 0) v_a = Dpi + v_a;

  //if(v_a < 0) v_a = Dpi - v_a;
  //if(v_a > Dpi) v_a -= Dpi;

  //v_t = atan( fabs(dY) * cos(v_a) / dX );
  char buf[80];
  std::sprintf(buf, "\nv_a: %+5.4f rad/%+5.2f deg\n", v_a, v_a*180/pi);

  std::cout << buf;

/*
  std::cout
    << "atan(0)     = " << atan(0)    << "\n"
    << "atan(1000)  = " << atan(1000) << "\n"
    << "atan(-1000) = " << atan(-1000) << "\n";
*/

  return 0;
}

