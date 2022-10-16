#include "gyro_data.h"

void Gyro_Init(void);
void Gyro_Read(u_int16_t);
void gyro_affiche(struct t_angle_gyro_double angle_gyro, char * titre);
struct t_angle_gyro_double gyro_get_angle(void);
struct t_angle_gyro_double gyro_get_vitesse(void);
int16_t gyro_get_temp(void);