#include "vectors.h"

#include <math.h>

GLfloat vec3_magnitude(struct vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

struct vec3 vec3_norm(struct vec3 v)
{
    GLfloat m = vec3_magnitude(v);
    if (m == 0)
        return v;
    return (struct vec3){ v.x / m, v.y / m, v.z / m };
}

struct vec3 vec3_neg(struct vec3 v)
{
    GLfloat m = vec3_magnitude(v);
    return (struct vec3){ -v.x, -v.y, -v.z };
}

struct vec3 vec3_sum(struct vec3 a, struct vec3 b)
{
    return (struct vec3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

struct vec3 vec3_diff(struct vec3 a, struct vec3 b)
{
    return (struct vec3){
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
}

GLfloat vec3_dot(struct vec3 a, struct vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct vec3 vec3_cross(struct vec3 a, struct vec3 b)
{
    return (struct vec3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}
