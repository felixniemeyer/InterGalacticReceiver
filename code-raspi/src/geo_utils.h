#ifndef GEO_UTILS_H
#define GEO_UTILS_H

#include "lib/q3dmath.h"

Matrix3 calc_cam_mat(Vector3 cam_pos, Vector3 look_at, double fov);
Matrix3 calc_rotx_mat(double theta);
Matrix3 calc_roty_mat(double theta);
Matrix3 calc_rotz_mat(double theta);
void mat_to_arr(const Matrix3 &mat, float *arr);

#endif
