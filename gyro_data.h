#include "pico/stdlib.h"

#ifndef GYRO_DATA_H
#define GYRO_DATA_H

struct t_angle_gyro_double{
    double rot_x, rot_y, rot_z;
};

struct t_angle_gyro{
    int32_t rot_x, rot_y, rot_z, temp;
};

#endif