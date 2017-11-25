#version 330
// атрибуты вершин базового элемента
in vec3 C3df;   // вершины
in vec2 TxCo;   // базовая техтура

// изменяемые атрибуты инстансов
in vec3 Origin; // положение центральной точки
in vec3 Norm;   // нормали вершин
in float Ring;   // карта яркости на границах

uniform mat4 mvp;
uniform float HalfSide; // Половина стороны инстанса
uniform vec3 Selected;  // Подсветка выделения

out vec2 f_texcoord;
out vec3 f_bright;  // R-G-B
out vec4 f_rings_sh;

void main(void)
{
	f_texcoord = TxCo;

	vec3 V = C3df + Norm * HalfSide;
	
	// Поворот на 90 градусов по оси Z
	if (Norm.x != 0.0f)
	{
		V.x -= HalfSide * sign(C3df.x);
		V.y -= HalfSide * Norm.x * sign(C3df.x);
	}

	// Поворот на 90 градусов по оси X
	if (Norm.z != 0.0f)
	{
		V.z -= HalfSide * sign(C3df.z);
		V.y -= HalfSide * Norm.z * sign(C3df.z);
	}

	// Переворот на 180 градусов по оси Z
	if (Norm.y == -1.0f)
	{
		V.x -= 2 * HalfSide * sign(C3df.x);
	}

	vec3 l_direct   = normalize(vec3(0.2f, 0.9f, 0.5f));    // вектор освещения
	vec3 l_bright   = vec3(0.16f, 0.16f, 0.2f);  // яркость источника
	vec3 mat_absorp = vec3(0.2f, 0.2f, 0.3f); // поглощение материалом

	if(Origin == Selected) mat_absorp = vec3(0.3f, 0.3f, 0.0f);

	// яркость освещения в формате RGBA
	f_bright = vec3(dot(l_direct, Norm)) * l_bright - mat_absorp;

	// яркость на границе объекта
	// Декодирование маски из троичной системы счисления
	f_rings_sh = vec4(0.0f);
		
	float bg = (f_bright[0] + f_bright[1] + f_bright[2])/3;
	float lt = (l_bright[0] +l_bright[1] +l_bright[2])/3;
	
	float shadow = 0.4 * (1 - 3*lt + bg);
	float reflex = -0.07;

	float r1, r2 = Ring, res;
	for(int i = 0; i < 3; ++i)
	{
		r1 = r2; r2 = trunc(r2/3);
		res = 1 - r1 + r2*3;
		if(res < 0) f_rings_sh[i] =  shadow;
		if(res > 0) f_rings_sh[i] = reflex;
	}
	res = 1 - r2;
	if(res < 0) f_rings_sh[3] =  shadow;
	if(res > 0) f_rings_sh[3] = reflex;

	gl_Position = mvp * vec4((V + Origin), 1.0f);
}
