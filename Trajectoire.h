#ifndef TRAJECTOIRE_H
#define TRAJECTOIRE_H

enum trajectoire_type_t{
    TRAJECTOIRE_DROITE,
    TRAJECTOIRE_CIRCULAIRE,
    TRAJECTOIRE_BEZIER
};

struct point_xy_t{
    double x, y;
};

struct trajectoire_t {
    enum trajectoire_type_t type;
    struct point_xy_t p1, p2, p3, p4;
    double orientation_debut, orientation_fin;
    double rayon, angle_debut_degre, angle_fin_degre;
    double longueur;
};

double Trajectoire_get_longueur_mm(struct trajectoire_t * trajectoire);
struct point_xy_t Trajectoire_get_point(struct trajectoire_t * trajectoire, double abscisse);
double Trajectoire_avance(struct trajectoire_t * trajectoire, double abscisse, double distance_mm);
double distance_points(struct point_xy_t point, struct point_xy_t point_old);
void Trajectoire_circulaire(struct trajectoire_t * trajectoire, double centre_x, double centre_y, double angle_debut_degre, double angle_fin_degre, double rayon);
void Trajectoire_droite(struct trajectoire_t * trajectoire, double p1_x, double p1_y, double p2_x, double p2_y);
void Trajectoire_bezier(struct trajectoire_t * trajectoire, double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y, double p4_x, double p4_y);

#endif
