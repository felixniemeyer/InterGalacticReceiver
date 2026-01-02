#include "geo_utils.h"

// Local dependencies

// Global
#include <math.h>

Matrix3 calc_cam_mat(Vector3 cam_pos, Vector3 look_at, double fov)
{
    const Vector3 cam_up = Vector3(0, 1, 0);

    double g = 0.5 / tan(fov / 2);
    Vector3 dir = look_at - cam_pos;
    dir.normalize();
    Vector3 right = dir.cross(cam_up);
    Vector3 up = right.cross(dir);

    // clang-format off
    return Matrix3(
        right.x, up.x, dir.x * g,
        right.y, up.y, dir.y * g,
        right.z, up.z, dir.z * g
    );
    // clang-format on
}

Matrix3 calc_rotx_mat(double theta)
{
    double s = sin(theta);
    double c = cos(theta);
    // clang-format off
    return Matrix3(
        1,  0,  0,
        0,  c, -s,
        0,  s,  c
    );
    // clang-format on
}

Matrix3 calc_roty_mat(double theta)
{
    double s = sin(theta);
    double c = cos(theta);
    // clang-format off
    return Matrix3(
         c,  0,  s,
         0,  1,  0,
        -s,  0,  c
    );
    // clang-format on
}

Matrix3 calc_rotz_mat(double theta)
{
    double s = sin(theta);
    double c = cos(theta);
    // clang-format off
    return Matrix3(
         c, -s,  0,
         s,  c,  0,
         0,  0,  1
    );
    // clang-format on
}

void mat_to_arr(const Matrix3 &mat, float *arr)
{
    arr[0] = mat.a;
    arr[1] = mat.b;
    arr[2] = mat.c;
    arr[3] = mat.d;
    arr[4] = mat.e;
    arr[5] = mat.f;
    arr[6] = mat.g;
    arr[7] = mat.h;
    arr[8] = mat.i;
}
