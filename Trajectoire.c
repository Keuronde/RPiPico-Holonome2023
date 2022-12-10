#include "Trajectoire.h"
#include "Trajectoire_bezier.h"
#include "Trajectoire_circulaire.h"
#include "Trajectoire_droite.h"

#include "math.h"

#define NB_MAX_TRAJECTOIRES 5
#define PRECISION_ABSCISSE 0.001


void Trajectoire_circulaire(struct trajectoire_t * trajectoire, double centre_x, double centre_y, double angle_debut_degre, double angle_fin_degre, double rayon){
    trajectoire->type = TRAJECTOIRE_CIRCULAIRE;
    trajectoire->p1.x = centre_x;
    trajectoire->p1.y = centre_y;
    trajectoire->angle_debut_degre = angle_debut_degre;
    trajectoire->angle_fin_degre = angle_fin_degre;
    trajectoire->rayon = rayon;
    trajectoire->longueur = -1;
}

void Trajectoire_droite(struct trajectoire_t * trajectoire, double p1_x, double p1_y, double p2_x, double p2_y){
    trajectoire->type = TRAJECTOIRE_DROITE;
    trajectoire->p1.x = p1_x;
    trajectoire->p1.y = p1_y;
    trajectoire->p2.x = p2_x;
    trajectoire->p2.y = p2_y;
    trajectoire->longueur = -1;
}

void Trajectoire_bezier(struct trajectoire_t * trajectoire, double p1_x, double p1_y, double p2_x, double p2_y, double p3_x, double p3_y, double p4_x, double p4_y){
    trajectoire->type = TRAJECTOIRE_BEZIER;
    trajectoire->p1.x = p1_x;
    trajectoire->p1.y = p1_y;
    trajectoire->p2.x = p2_x;
    trajectoire->p2.y = p2_y;
    trajectoire->p3.x = p3_x;
    trajectoire->p3.y = p3_y;
    trajectoire->p4.x = p4_x;
    trajectoire->p4.y = p4_y;
    trajectoire->longueur = -1;
}

/// @brief Renvoie la longueur de la trajectoire en mm, la calcule si besoin
/// @param trajectoire 
/// @return Longueur de la trajectoire
double Trajectoire_get_longueur_mm(struct trajectoire_t * trajectoire){
    if(trajectoire->longueur > 0){
        // La longueur est déjà calculée
    }else{
        // Calculons la longueur de la trajectoire
        switch(trajectoire->type){
            case TRAJECTOIRE_DROITE:
                Trajectoire_droite_get_longueur(trajectoire);
                break;
            case TRAJECTOIRE_CIRCULAIRE:
                Trajectoire_circulaire_get_longueur(trajectoire);
                break;
            
            case TRAJECTOIRE_BEZIER:
                Trajectoire_bezier_get_longueur(trajectoire);
                break;
        }
    }
    return trajectoire->longueur;
}

/// @brief Renvoie le point d'une trajectoire à partir de son abscisse
/// @param abscisse : abscisse sur la trajectoire
/// @return point en coordonnées X/Y
struct point_xy_t Trajectoire_get_point(struct trajectoire_t * trajectoire, double abscisse){
    switch(trajectoire->type){
        case TRAJECTOIRE_DROITE:
            return Trajectoire_droite_get_point(trajectoire, abscisse);
            break;

        case TRAJECTOIRE_CIRCULAIRE:
            return Trajectoire_circulaire_get_point(trajectoire, abscisse);
            break;
        
        case TRAJECTOIRE_BEZIER:
            return Trajectoire_bezier_get_point(trajectoire, abscisse);
            break;
    }
    
}

/// @brief Calcul la nouvelle abscisse une fois avancé de la distance indiquée
/// @param abscisse : Valeur entre 0 et 1, position actuelle du robot sur sa trajectoire
/// @param distance_mm : Distance en mm de laquelle le robot doit avancer sur la trajectoire
/// @return nouvelle abscisse
double Trajectoire_avance(struct trajectoire_t * trajectoire, double abscisse, double distance_mm){
    double delta_abscisse, delta_mm, erreur_relative;

    if(distance_mm == 0){
        return abscisse;
    }
    // Ceci permet d'avoir une abscisse exact sur les trajectoires droites et les trajectoires circulaires
    delta_abscisse = distance_mm / Trajectoire_get_longueur_mm(trajectoire);
    delta_mm = distance_points(Trajectoire_get_point(trajectoire, abscisse), Trajectoire_get_point(trajectoire, abscisse + delta_abscisse) );

    // Sur les trajectoires de bézier, il peut être nécessaire d'affiner 
    // Les cas où l'algorythme diverge ne devraient pas se produire car distance_cm << longeur_trajectoire.
    erreur_relative = 1 - delta_mm / distance_mm;
    while(fabs(erreur_relative) > PRECISION_ABSCISSE){
        delta_abscisse = delta_abscisse * distance_mm / delta_mm;
        delta_mm = distance_points(Trajectoire_get_point(trajectoire, abscisse), Trajectoire_get_point(trajectoire, abscisse + delta_abscisse) );
        erreur_relative = 1 - delta_mm / distance_mm;
    }
    
    return abscisse + delta_abscisse;
}

double distance_points(struct point_xy_t point, struct point_xy_t point_old){
    return sqrt( pow(point.x - point_old.x, 2) + pow(point.y - point_old.y , 2));
    
}
