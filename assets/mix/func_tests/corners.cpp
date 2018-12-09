/*
     P
    |\ c
   a| \
    |__\V
    b  (bc) -  угол между "b" и "с" от -90 (-Hpi) до 90 (Hpi)

  double bc = atan(a/b);            // угол между b-c
  double c = fabs(b) /cos(bc);      // длина с
*/

#include <iostream>
#include <cmath>

double
  Hpi = acos(0),
  pi  = 2 * Hpi,
  Dpi = 2 * pi;

// вычисляет углы направления от камеры до указанной вершины меша
void angles_calc( double Vx, double Vy, double Vz )
{
  // положение камеры
  double P[] = { 0.0, 2.0, 0.0 };
  double
    dX = P[0] - Vx,
    dY = P[1] - Vy,
    dZ = P[2] - Vz,
    v_a, v_t;

  if(0.0 == dX) dX -= 0.000001; // потому что
  if(0.0 == dY) dY -= 0.000001; // нельзя
  if(0.0 == dZ) dZ -= 0.000001; // делить на ноль

  v_a = atan(dZ / dX);
  if(dX > 0) v_a -= pi;
  if(v_a < 0) v_a = Dpi + v_a;


  v_t = atan( dY * cos(v_a) / dX );

  char buf[80];
  std::sprintf(buf, "v_a: %+5.4f rad (%6.2f deg), v_t: %+5.4f rad (%6.2f deg)\n",
    v_a, v_a*180/pi, v_t, v_t*180/pi);
  std::cout << buf;

  return;
}

int main(int, char**)
{
std::cout << "\n";
angles_calc(  0.0, 0.0,  0.0 );
angles_calc(  2.0, 0.0,  0.0 );
angles_calc(  2.0, 0.0,  2.0 );
angles_calc(  0.0, 0.0,  2.0 );
angles_calc( -2.0, 0.0,  2.0 );
angles_calc( -2.0, 0.0,  0.0 );
angles_calc( -2.0, 0.0, -2.0 );
angles_calc(  0.0, 0.0, -2.0 );
angles_calc(  2.0, 0.0, -2.0 );
angles_calc(  2.0, 0.0, -0.00001 );


/*
  std::cout
    << "atan(0)     = " << atan(0)    << "\n"
    << "atan(1000)  = " << atan(1000) << "\n"
    << "atan(-1000) = " << atan(-1000) << "\n";
*/

  return 0;
}

