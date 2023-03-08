#include "mat4.h"
#include <stdio.h>
#include <string.h>
void create_matrix_identity(Matrix4x4 *matrix)
{
    Matrix4x4 val = {{{1, 0, 0, 0},
                      {0, 1, 0, 0},
                      {0, 0, 1, 0},
                      {0, 0, 0, 1}}};

    *matrix = val;
}

void create_matrix_scale(Matrix4x4 *matrix, float x, float y, float z)
{
    Matrix4x4 val = {{{x, 0, 0, 0},
                      {0, y, 0, 0},
                      {0, 0, z, 0},
                      {0, 0, 0, 1}}};

    *matrix = val;
}

void create_matrix_translate(Matrix4x4 *matrix, float x, float y, float z)
{
    Matrix4x4 val = {{{1, 0, 0, x},
                      {0, 1, 0, y},
                      {0, 0, 1, z},
                      {0, 0, 0, 1}}};

    *matrix = val;
}
void create_matrix_rotate_x(Matrix4x4 *matrix, float angle)
{
    Matrix4x4 val = {{{1, 0, 0, 0},
                      {0, cos(angle), -sin(angle), 0},
                      {0, sin(angle), cos(angle), 0},
                      {0, 0, 0, 1}}};

    *matrix = val;
}
void create_matrix_rotate_y(Matrix4x4 *matrix, float angle)
{
    Matrix4x4 val = {{{cos(angle), 0, sin(angle), 0},
                      {0, 1, 0, 0},
                      {-sin(angle), 0, cos(angle), 0},
                      {0, 0, 0, 1}}};

    *matrix = val;
}

void create_matrix_rotate_z(Matrix4x4 *matrix, float angle)
{
    // rotate a matrix around the z axis
    Matrix4x4 val = {{{cos(angle), -sin(angle), 0, 0},
                      {sin(angle), cos(angle), 0, 0},
                      {0, 0, 1, 0},
                      {0, 0, 0, 1}}};

    *matrix = val;
}

void create_matrix_rotate_q(Matrix4x4 *matrix, double x, double y, double z, double w)
{
    // rotate a matrix with a quaternion

    create_matrix_identity(matrix);

    double sx = x * x;
    double sy = y * y;
    double sz = z * z;
    double sw = w * w;

    if (fabs(sx + sy + sz + sw) <= 0.0001f)
    {
        return;
    }

    double inv_q = 1.0 / (sx + sy + sz + sw);
    if (fabs(sx + sy + sz + sw - 1.0) <= 0.0001f)
    {
        inv_q = 1.0;
    }

    Matrix4x4 val = {{{(sx - sy - sz + sw) * inv_q, (2 * (x * y - z * w)) * inv_q, (2 * (x * z + y * w)) * inv_q, 0},
                      {(2 * (x * y + z * w)) * inv_q, (sy - sx - sz + sw) * inv_q, (2 * (y * z - x * w)) * inv_q, 0},
                      {(2 * (x * z - y * w)) * inv_q, (2 * (y * z + x * w) * inv_q), (-sx - sy + sz + sw) * inv_q, 0},
                      {0, 0, 0, 1}}};
    *matrix = val;
}

static void matrix_getcofactor(const Matrix4x4 *matrix, Matrix4x4 *result, int r, int c, int size)
{
    int rx = 0;
    int ry = 0;
    int x, y = 0;
    for (x = 0; x < size; x++)
    {
        for (y = 0; y < size; y++)
        {
            if (x != r && y != c)
            {
                result->value[ry][rx++] = matrix->value[y][x];
                if (rx == size - 1)
                {
                    rx = 0;
                    ry++;
                }
            }
        }
    }
}

static float matrix_get_determinant(const Matrix4x4 *matrix, int size)
{
    float det = 0;
    Matrix4x4 cofactor;
    int i;

    float sign = 1;

    if (size == 1)
    {
        return matrix->value[0][0];
    }

    for (i = 0; i < size; i++)
    {
        matrix_getcofactor(matrix, &cofactor, 0, i, size);
        det += sign * matrix->value[i][0] * matrix_get_determinant(&cofactor, size - 1);

        sign = -sign;
    }
    return det;
}
static void matrix_get_adjoint(const Matrix4x4 *matrix, Matrix4x4 *result)
{
    float sign;

    Matrix4x4 temp_matrix = {};
    int x, y;
    for (x = 0; x < 4; x++)
    {
        for (y = 0; y < 4; y++)
        {
            matrix_getcofactor(matrix, &temp_matrix, x, y, 4);
            sign = ((x + y) % 2 == 0) ? 1.0 : -1.0;
            result->value[y][x] = sign * matrix_get_determinant(&temp_matrix, 4 - 1);
        }
    }
}
void matrix_inverse(const Matrix4x4 *matrix, Matrix4x4 *result)
{
    // calculate the inverse of the 4x4 matrix
    // using Cramer's Rule
    int x, y;
    Matrix4x4 adjoint;
    float det = matrix_get_determinant(matrix, 4);

    if (det <= 0.0f)
    {
        create_matrix_identity(result);
        // printf("matrix_inverse: determinant is 0.0f\n");
        return;
    }

    matrix_get_adjoint(matrix, &adjoint);

    for (x = 0; x < 4; x++)
    {
        for (y = 0; y < 4; y++)
        {
            result->value[y][x] = adjoint.value[y][x] / det;
        }
    }

    // TODO: check if this is correct we may need to do a transpose !
}

void matrix_transpose(const Matrix4x4 *matrix, Matrix4x4 *result)
{
    // transpose the matrix
    result->value[0][0] = matrix->value[0][0];
    result->value[0][1] = matrix->value[1][0];
    result->value[0][2] = matrix->value[2][0];
    result->value[0][3] = matrix->value[3][0];
    result->value[1][0] = matrix->value[0][1];
    result->value[1][1] = matrix->value[1][1];
    result->value[1][2] = matrix->value[2][1];
    result->value[1][3] = matrix->value[3][1];
    result->value[2][0] = matrix->value[0][2];
    result->value[2][1] = matrix->value[1][2];
    result->value[2][2] = matrix->value[2][2];
    result->value[2][3] = matrix->value[3][2];
    result->value[3][0] = matrix->value[0][3];
    result->value[3][1] = matrix->value[1][3];
    result->value[3][2] = matrix->value[2][3];
    result->value[3][3] = matrix->value[3][3];
}
void matrix_multiply(const Matrix4x4 *a, const Matrix4x4 *b, Matrix4x4 *result)
{
    int x, y;
    Matrix4x4 acopy = *a;
    Matrix4x4 bcopy = *b;
    for (x = 0; x < 4; x++)
    {
        for (y = 0; y < 4; y++)
        {
            result->value[y][x] = acopy.value[y][0] * bcopy.value[0][x] +
                                  acopy.value[y][1] * bcopy.value[1][x] +
                                  acopy.value[y][2] * bcopy.value[2][x] +
                                  acopy.value[y][3] * bcopy.value[3][x];
        }
    }
}

void matrix_apply_vector(const Matrix4x4 *matrix, Vec3 *vector)
{
    Vec3 temp = {};
    temp.x = vector->x * matrix->value[0][0] + vector->y * matrix->value[0][1] + vector->z * matrix->value[0][2];
    temp.y = vector->x * matrix->value[1][0] + vector->y * matrix->value[1][1] + vector->z * matrix->value[1][2];
    temp.z = vector->x * matrix->value[2][0] + vector->y * matrix->value[2][1] + vector->z * matrix->value[2][2];
    temp._padding = vector->_padding;
    *vector = temp;
}

// point is just apply vector with the matrix offset applied
void matrix_apply_point(const Matrix4x4 *matrix, Vec3 *point)
{
    Vec3 temp = {};
    temp.x = point->x * matrix->value[0][0] + point->y * matrix->value[0][1] + point->z * matrix->value[0][2] + matrix->value[0][3];
    temp.y = point->x * matrix->value[1][0] + point->y * matrix->value[1][1] + point->z * matrix->value[1][2] + matrix->value[1][3];
    temp.z = point->x * matrix->value[2][0] + point->y * matrix->value[2][1] + point->z * matrix->value[2][2] + matrix->value[2][3];
    *point = temp;
}
Vec3 matrix_apply_vector_ret(const Matrix4x4 *matrix, Vec3 vector)
{
    Vec3 c = vector;
    matrix_apply_vector(matrix, &c);
    return c;
}
Vec3 matrix_apply_point_ret(const Matrix4x4 *matrix, Vec3 point)
{
    Vec3 c = point;
    matrix_apply_point(matrix, &c);
    return c;
}

Matrix4x4 matrix_lookat(Vec3 pos, Vec3 center, Vec3 up)
{
    Vec3 f = vec3_unit(vec3_sub(center, pos));
    Vec3 s = vec3_unit(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Matrix4x4 result = {};
    create_matrix_identity(&result);

    result.value[0][0] = s.x;
    result.value[1][0] = s.y;
    result.value[2][0] = s.z;
    result.value[0][1] = u.x;
    result.value[1][1] = u.y;
    result.value[2][1] = u.z;
    result.value[0][2] = -f.x;
    result.value[1][2] = -f.y;
    result.value[2][2] = -f.z;
    result.value[3][3] = 1.0f;

    result.value[3][0] = -vec3_dot(s, pos);
    result.value[3][1] = -vec3_dot(u, pos);
    result.value[3][2] = vec3_dot(f, pos);

    return result;
}

Matrix4x4 matrix_perspective(float fov_deg, float ratio, float near, float far)
{
    float fov_rad = fov_deg * (M_PI / 180.0f);
    float f = 1.0f / tan(fov_rad / 2.0f);
    Matrix4x4 result = {};
    create_matrix_identity(&result);


    result.value[0][0] = f / ratio;
    result.value[1][1] = f;
    result.value[2][2] = (far + near) / (near - far);
    result.value[2][3] = -1.0f;
    result.value[3][2] = (-2.0f * far * near) / (far - near);
    result.value[3][3] = 1.0f;
    return result;
}
