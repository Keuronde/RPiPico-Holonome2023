#include "gyro_data.h"

int gyro_init_check();
int gyro_config();
void gyro_get_vitesse_brute(struct t_angle_gyro* angle_gyro, struct t_angle_gyro* angle_gyro_moy);
void gyro_get_vitesse_normalisee(struct t_angle_gyro* _vitesse_angulaire, struct t_angle_gyro_double * vitesse_gyro);
