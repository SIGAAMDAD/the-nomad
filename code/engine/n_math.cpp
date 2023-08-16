#include "code/engine/n_shared.h"

// c++ version of the stuff found in mathlib.c

const vec4_t		colorBlack	= {0, 0, 0, 1};
const vec4_t		colorRed	= {1, 0, 0, 1};
const vec4_t		colorGreen	= {0, 1, 0, 1};
const vec4_t		colorBlue	= {0, 0, 1, 1};
const vec4_t		colorYellow	= {1, 1, 0, 1};
const vec4_t		colorMagenta= {1, 0, 1, 1};
const vec4_t		colorCyan	= {0, 1, 1, 1};
const vec4_t		colorWhite	= {1, 1, 1, 1};
const vec4_t		colorLtGrey	= {0.75, 0.75, 0.75, 1};
const vec4_t		colorMdGrey	= {0.5, 0.5, 0.5, 1};
const vec4_t		colorDkGrey	= {0.25, 0.25, 0.25, 1};


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

float disBetweenOBJ(const glm::vec3& src, const glm::vec3& tar)
{
	if (src.y == tar.y) // horizontal
		return src.x > tar.x ? (src.x - tar.x) : (tar.x - src.x);
	else if (src.x == tar.x) // vertical
		return src.y > tar.y ? (src.y - tar.y) : (tar.y - src.y);
	else // diagonal
		return Q_root((pow((src.x - tar.x), 2) + pow((src.y - tar.y), 2)));
}

float disBetweenOBJ(const glm::vec2& src, const glm::vec2& tar)
{
	if (src.y == tar.y) // horizontal
		return src.x > tar.x ? (src.x - tar.x) : (tar.x - src.x);
	else if (src.x == tar.x) // vertical
		return src.y > tar.y ? (src.y - tar.y) : (tar.y - src.y);
	else // diagonal
		return Q_root((pow((src.x - tar.x), 2) + pow((src.y - tar.y), 2)));
}

float disBetweenOBJ(const vec2_t src, const vec2_t tar)
{
	if (src[1] == tar[1]) // horizontal
		return src[0] > tar[0] ? (src[0] - tar[0]) : (tar[0] - src[0]);
	else if (src[0] == tar[0]) // vertical
		return src[1] > tar[1] ? (src[1] - tar[1]) : (tar[1] - src[1]);
	else // diagonal
		return Q_root((pow((src[0] - tar[0]), 2) + pow((src[1] - tar[1]), 2)));
}

float disBetweenOBJ(const coord_t& src, const coord_t& tar)
{
	if (src.y == tar.y) // horizontal
		return src.x > tar.x ? (src.x - tar.x) : (tar.x - src.x);
	else if (src.x == tar.x) // vertical
		return src.y > tar.y ? (src.y - tar.y) : (tar.y - src.y);
	else // diagonal
		return Q_root((pow((src.x - tar.x), 2) + pow((src.y - tar.y), 2)));
}

#if 0
vec_t VectorNormalize(vec3_t v)
{
	float ilength, length;

	length = DotProduct(v, v);

	if (length) {
#if 1
		// writing it this way allows g++ to recognize that rsqrt can be used
		ilength = 1/(float)sqrt(length);
#else
		ilength = Q_rsqrt(length);
#endif
		length *= ilength;

		v[0] *= ilength;
		v[1] *= ilength;
	}
	return length;
}

vec_t VectorNormalize(glm::vec3& v)
{
	float ilength, length;

	length = DotProduct(v, v);

	if (length) {
#if 1
		// writing it this way allows g++ to recognize that rsqrt can be used
		ilength = 1/(float)sqrt(length);
#else
		ilength = Q_rsqrt(length);
#endif
		length *= ilength;

		v[0] *= ilength;
		v[1] *= ilength;
	}
	return length;
}
#endif
