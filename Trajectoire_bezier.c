#include "Trajectoire.h"
#include "Trajectoire_bezier.h"


void Trajectoire_bezier_get_longueur(struct trajectoire_t * trajectoire){
    struct point_xy_t point, point_old;
    double nb_pas=500;

    trajectoire->longueur=0;
    point_old = trajectoire->p1;

    for(double abscisse=0; abscisse<=1; abscisse += 1./nb_pas){
        point = Trajectoire_bezier_get_point(trajectoire, abscisse);
        trajectoire->longueur += distance_points(point, point_old);
        point_old = point;
    }
}


/// @brief Retourne le point sur la trajectoire en fonction de l'abscisse
/// @param abscisse : compris entre 0 et 1
struct point_xy_t Trajectoire_bezier_get_point(struct trajectoire_t * trajectoire, double abscisse){
    struct point_xy_t point;
    point.x = (double) trajectoire->p1.x * (1-abscisse) * (1-abscisse) * (1-abscisse) + 
        3 * (double) trajectoire->p2.x * abscisse * (1-abscisse) * (1-abscisse) +
        3 * (double) trajectoire->p3.x * abscisse * abscisse * (1-abscisse) +
            (double) trajectoire->p4.x * abscisse * abscisse * abscisse;
                    
    point.y = (double) trajectoire->p1.y * (1-abscisse) * (1-abscisse) * (1-abscisse) + 
        3 * (double) trajectoire->p2.y * abscisse * (1-abscisse) * (1-abscisse) +
        3 * (double) trajectoire->p3.y * abscisse * abscisse * (1-abscisse) +
            (double) trajectoire->p4.y * abscisse * abscisse * abscisse;

    return point;
}