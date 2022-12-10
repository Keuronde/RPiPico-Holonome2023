#include "math.h"
#include "Trajectoire.h"


void Trajectoire_circulaire_get_longueur(struct trajectoire_t * trajectoire){
    double distance_angulaire;
    if(trajectoire->angle_debut_degre > trajectoire->angle_fin_degre){
        distance_angulaire = trajectoire->angle_debut_degre - trajectoire->angle_fin_degre;
    }else{
        distance_angulaire = trajectoire->angle_fin_degre - trajectoire->angle_debut_degre;
    }
    trajectoire->longueur = 2. * M_PI * trajectoire->rayon * distance_angulaire / 360.;
}

/// @brief Retourne le point sur la trajectoire en fonction de l'abscisse
/// @param abscisse : compris entre 0 et 1
struct point_xy_t Trajectoire_circulaire_get_point(struct trajectoire_t * trajectoire, double abscisse){
    struct point_xy_t point;
    double angle_degre;

    angle_degre = (double) trajectoire->angle_debut_degre * (1-abscisse) + (double) trajectoire->angle_fin_degre * abscisse;
    point.x = trajectoire->p1.x + cos(angle_degre/180. * M_PI) * trajectoire->rayon;
    point.y = trajectoire->p1.y + sin(angle_degre/180. * M_PI) * trajectoire->rayon;

    return point;
}
