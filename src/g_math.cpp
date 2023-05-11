#include "n_shared.h"

constexpr float threehalfs = 1.5f;

float Q_root(float x)
{
	int64_t     i;								// The integer interpretation of x
	float       x_half = x * 0.5f;
	float       r_sqrt = x;

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

    x2 = number * 0.5F;
    x = *(long *)&number;
    x = 0x5f3759df - (x >> 1);
    y = *(float *)&x;
    y = y * ( threehalfs - ( x2 * y * y ) );
//  y = y * ( threehalfs - ( x2 * y * y ) );

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

int32_t disBetweenOBJ(const coord_t& src, const coord_t& tar)
{
	if (src.y == tar.y) // horizontal
		return src.x > tar.x ? (src.x - tar.x) : (tar.x - src.x);
	else if (src.x == tar.x) // vertical
		return src.y > tar.y ? (src.y - tar.y) : (tar.y - src.y);
	else // diagonal
		return Q_root((pow((src.x - tar.x), 2) + pow((src.y - tar.y), 2)));
}