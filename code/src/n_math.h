#ifndef _N_MATH_
#define _N_MATH_

#pragma once

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t mat3_t[3][3];
typedef vec_t mat4_t[4][4];
typedef unsigned char byte;

extern const vec3_t vec3_origin;
extern const vec2_t vec2_origin;

#define VectorAdd(a,b,c) ((c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];)
#define VectorSubtract(a,b,c) ((c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];)
#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1])
#define VectorCopy(x,y) ((x)[0]=(y)[0];(x)[1]=(y)[1];)
#define VectorScale(in,scale,out) ((out)[0]=(in)[0]*(scale);(out)[1]=(in)[1]*(scale);)
#define VectorClear(x) ((x)[0]=0;(x)[1]=0;)

#ifndef Q3_VM
void CrossProduct(const vec2_t v1, const vec2_t v2, vec2_t out);
#else
GDR_INLINE CrossProduct(const vec2_t v1, const vec2_t v2, vec2_t cross)
{
    cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
}
GDR_INLINE CrossProduct(const glm::vec2& v1, const glm::vec2& v2, glm::vec2& cross)
{
    cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
}
GDR_INLINE CrossProduct(const glm::vec3& v1, const glm::vec3& v2, glm::vec3& cross)
{
    cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
}
#endif

float disBetweenOBJ(const vec2_t src, const vec2_t tar);
float Q_rsqrt(float number);
float Q_root(float x);

#endif