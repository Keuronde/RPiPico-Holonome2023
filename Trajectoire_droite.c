#include "Trajectoire.h"


void Trajectoire_droite_get_longueur(struct trajectoire_t * trajectoire){
    trajectoire->longueur = distance_points(trajectoire->p1, trajectoire->p2);
}

/// @brief Retourne le point sur la trajectoire en fonction de l'abscisse
/// @param abscisse : compris entre 0 et 1
struct point_xy_t Trajectoire_droite_get_point(struct trajectoire_t * trajectoire, double abscisse){
    struct point_xy_t point;

    point.x = (double) trajectoire->p1.x * (1. - abscisse) + (double) trajectoire->p2.x * abscisse;
    point.y = (double) trajectoire->p1.y * (1. - abscisse) + (double) trajectoire->p2.y * abscisse;
    
    return point;
}