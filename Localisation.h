struct position_t{
    double x_mm, y_mm;
    double angle_radian;
};

struct position_t Localisation_get(void);
void Localisation_gestion();
void Localisation_init();
