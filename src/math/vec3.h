#pragma once 
#include <math.h>
#include <stdbool.h>
#include <math/fast.h>

typedef struct vec3_t
{
    float x;
    float y;
    float z;
    float _padding;
} __attribute__((packed)) Vec3;
static inline Vec3 vec3_create(float x, float y, float z)
{
    Vec3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

#define vec3$(X,Y,Z) (vec3_create((X), (Y), (Z)))

static inline Vec3 vec3_inv(Vec3 vec)
{
    return vec3_create(-vec.x, -vec.y, -vec.z);
}

static inline float vec3_squared_length(Vec3 vec)
{
    return (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

static inline float vec3_length(Vec3 vec)
{
    return sqrtf(vec3_squared_length(vec));
}

static inline Vec3 vec3_add(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}

static inline Vec3 vec3_sub(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
}

static inline Vec3 vec3_min(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(fminf(vec1.x, vec2.x), fminf(vec1.y, vec2.y), fminf(vec1.z, vec2.z));
}

static inline Vec3 vec3_max(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(fmaxf(vec1.x, vec2.x), fmaxf(vec1.y, vec2.y), fmaxf(vec1.z, vec2.z));
}
static inline float vec3_min_comp(Vec3 vec1)
{
    return fminf(vec1.x, fminf(vec1.y, vec1.z));
}

static inline float vec3_max_comp(Vec3 vec1)
{
    return fmaxf(vec1.x, fmaxf(vec1.y, vec1.z));
}
#ifdef USE_INTRINSIC
#    include <immintrin.h>
#    include <x86intrin.h>

static inline Vec3 vec3_div(Vec3 vec1, Vec3 vec2)
{

    __m128 a = _mm_load_ps((float *)&vec1);
    __m128 b = _mm_load_ps((float *)&vec2);

    __m128 res_m = _mm_div_ps(a, b);
    Vec3 res = {};
    _mm_store_ps((float *)&res, res_m);

    return res;
}

static inline Vec3 vec3_mul(Vec3 vec1, Vec3 vec2)
{
    __m128 a = _mm_load_ps((float *)&vec1);
    __m128 b = _mm_load_ps((float *)&vec2);

    __m128 res_m = _mm_mul_ps(a, b);
    Vec3 res = {};
    _mm_store_ps((float *)&res, res_m);

    return res;
    return vec3_create(vec1.x * vec2.x, vec1.y * vec2.y, vec1.z * vec2.z);
}
static inline Vec3 vec3_cross(Vec3 vec1, Vec3 vec2)
{

    __m128 a = _mm_load_ps((float *)&vec1);
    __m128 b = _mm_load_ps((float *)&vec2);
    __m128 tmp0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 tmp2 = _mm_mul_ps(tmp0, b);
    __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
    __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 res_m = _mm_sub_ps(tmp3, tmp4);
    Vec3 res = {};
    _mm_store_ps((float *)&res, res_m);

    return res;
}

#else

static inline Vec3 vec3_cross(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
}
static inline Vec3 vec3_div(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(vec1.x / vec2.x, vec1.y / vec2.y, vec1.z / vec2.z);
}

static inline Vec3 vec3_mul(Vec3 vec1, Vec3 vec2)
{
    return vec3_create(vec1.x * vec2.x, vec1.y * vec2.y, vec1.z * vec2.z);
}
#endif
static inline Vec3 vec3_mul_val(Vec3 vec1, float x)
{
    return vec3_create(vec1.x * x, vec1.y * x, vec1.z * x);
}

static inline Vec3 vec3_div_val(Vec3 vec1, float x)
{
    return vec3_create(vec1.x / x, vec1.y / x, vec1.z / x);
}

static inline float vec3_dot(Vec3 vec1, Vec3 vec2)
{

    return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
}
static inline bool is_vec3_near_zero(Vec3 vec)
{
    const float precision = 1e-8;

    return (fabs((double)vec.x) < precision) && (fabs((double)vec.y) < precision) && (fabs((double)vec.z) < precision);
}
static inline bool is_vec3_near_zero_l(Vec3 vec)
{
    const float precision = 1e-5;
    return (fabs((double)vec.x) < precision) && (fabs((double)vec.y) < precision) && (fabs((double)vec.z) < precision);
}

static inline bool vec3_eq(Vec3 l, Vec3 r)
{
    return is_vec3_near_zero_l(vec3_sub(l, r));
}

/* vec / sqrt(vecx * vecx + vecy * vecy + vecz * vecz)
// vec * 1/sqrt()
// vec * Q_rdqrt()*/
static inline Vec3 vec3_unit(Vec3 vec)
{
    return vec3_mul_val(vec, Q_rsqrt(vec3_squared_length(vec)));
}

/* todo: implement way better random_vec3_x because this doesn't look good */

static inline Vec3 reflect(Vec3 vec1, Vec3 vec2)
{
    return vec3_sub(vec1, vec3_mul_val(vec2, (vec3_dot(vec1, vec2) * 2.0)));
}

__attribute__((hot)) static inline float vec_axis(Vec3 vec, int axis)
{
    if (axis == 0)
    {
        return vec.x;
    }
    else if (axis == 1)
    {
        return vec.y;
    }
    return vec.z;
}



