#ifndef MATRIX_H
#define MATRIX_H
#include "utils.h"
#include "vec3.h"
typedef struct
{
    float value[4][4];
} Matrix4x4;

void create_matrix_identity(Matrix4x4 *matrix);

void create_matrix_scale(Matrix4x4 *matrix, float x, float y, float z);
void create_matrix_translate(Matrix4x4 *matrix, float x, float y, float z);
void create_matrix_rotate_x(Matrix4x4 *matrix, float angle);
void create_matrix_rotate_y(Matrix4x4 *matrix, float angle);
void create_matrix_rotate_z(Matrix4x4 *matrix, float angle);
void create_matrix_rotate_q(Matrix4x4 *matrix, double x, double y, double z, double w);

void matrix_inverse(const Matrix4x4 *matrix, Matrix4x4 *result);
void matrix_transpose(const Matrix4x4 *matrix, Matrix4x4 *result);
void matrix_multiply(const Matrix4x4 *a, const Matrix4x4 *b, Matrix4x4 *result);

void matrix_apply_vector(const Matrix4x4 *matrix, Vec3 *vector);
void matrix_apply_point(const Matrix4x4 *matrix, Vec3 *point);
Vec3 matrix_apply_vector_ret(const Matrix4x4 *matrix, Vec3 vector);
Vec3 matrix_apply_point_ret(const Matrix4x4 *matrix, Vec3 point);

#endif /* FE439C8D_4B4A_492C_81C7_2F76373A7F98 */
