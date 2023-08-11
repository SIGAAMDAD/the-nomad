#include "n_shared.h"
#include <cglm/cglm.h>

// c++ version of the stuff found in mathlib.c

float Q_root(float x)
{
	long        i;								// The integer interpretation of x
	float       x_half = x * 0.5f;
	float       r_sqrt = x;
	const float threehalfs = 1.5F;

	// trick c/c++, bit hack
	i = *(int64_t *)&r_sqrt;					    // oh yes, undefined behaviour, who gives a fuck?
	i = 0x5f375a86 - (i >> 1);				            // weird magic base-16 nums
	r_sqrt = *(float *) &i;

	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 1st Newton iteration
	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 2nd Newton iteration

	return x * r_sqrt; // x * (1/sqrt(x)) := sqrt(x)
}

float Q_rsqrt(float number)
{
    long x;
    float x2, y;
	const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    x = *(long *)&number;                    // evil floating point bit level hacking
    x = 0x5f3759df - (x >> 1);               // what the fuck?
    y = *(float *)&x;
    y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
//  y = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

    return y;
}

void MatrixRotate(const mat4_t m, mat4_t o, float radians, vec3_t v)
{
	const float a = radians;
	const float c = cos(a);
	const float s = sin(a);
	uint32_t
	vec3_t axis, temp;
	mat4_t rotate;

	VectorCopy(axis, v);
	VectorNormalize(axis);
	VectorCopy(temp, axis);

	temp[0] = 1.0f - c * axis[0];
	temp[1] = 1.0f - c * axis[1];
	temp[2] = 1.0f - c * axis[2];

	rotate[0][0] = c + temp[0] * axis[0];
	rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	rotate[1][1] = c + temp[1] * axis[1];
	rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	rotate[2][2] = c + temp[2] * axis[2];

	o[0] = m[0] * rotate[0][0] + m[1] * rotate[0][1] + m[2] * rotate[0][2];
	o[1] = m[0] * rotate[1][0] + m[1] * rotate[1][1] + m[2] * rotate[1][2];
	o[2] = m[0] * rotate[2][0] + m[1] * rotate[2][1] + m[2] * rotate[2][2];
	o[3] = m[3];
}

/*
disBetweenOBJ: z coordinates are unused
*/
float disBetweenOBJ(const vec3_t src, const vec3_t tar)
{
	if (src[1] == tar[1]) // horizontal
		return src[0] > tar[0] ? (src[0] - tar[0]) : (tar[0] - src[0]);
	else if (src[0] == tar[0]) // vertical
		return src[1] > tar[1] ? (src[1] - tar[1]) : (tar[1] - src[1]);
	else // diagonal
		return Q_root((pow((src[0] - tar[0]), 2) + pow((src[1] - tar[1]), 2)));
}
