struct t_angle_gyro_double{
    double rot_x, rot_y, rot_z;
};

void Gyro_Init(void);
void Gyro_Read(u_int16_t);
void gyro_affiche(struct t_angle_gyro_double angle_gyro, char * titre);
struct t_angle_gyro_double gyro_get_angle(void);