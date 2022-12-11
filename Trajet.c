#include <math.h>
#include "Geometrie.h"
#include "Trajectoire.h"
#include "Asser_Position.h"

#define VITESSE_MAX_MM_S 1000

double Trajet_calcul_vitesse(double temps_s);

double abscisse;
double position_mm; // Position en mm sur la trajectoire
double vitesse_mm_s;
double acceleration_mm_ss;
struct trajectoire_t trajet_trajectoire;
struct position_t position_consigne;

void Trajet_init(){
    abscisse = 0;
    vitesse_mm_s = 0;
    acceleration_mm_ss = 300;
    position_mm = 0;
}

void Trajet_debut_trajectoire(struct trajectoire_t trajectoire){
    abscisse = 0;
    vitesse_mm_s = 0;
    acceleration_mm_ss = 1500;
    position_mm = 0;
    trajet_trajectoire = trajectoire;
}

void Trajet_avance(double pas_de_temps_s){
    double distance_mm, orientation_radian;
    struct point_xy_t point;
    struct position_t position;

    // Calcul de la vitesse 
    vitesse_mm_s = Trajet_calcul_vitesse(pas_de_temps_s);

    // Calcul de l'avancement en mm
    distance_mm = vitesse_mm_s * pas_de_temps_s;
    position_mm += distance_mm;

    // Calcul de l'abscisse sur la trajectoire
    abscisse = Trajectoire_avance(&trajet_trajectoire, abscisse, distance_mm);
    
    // Obtention du point consigne
    point = Trajectoire_get_point(&trajet_trajectoire, abscisse);

    // Obtention de l'orientation consigne
    orientation_radian = 0; // TODO

    position.x_mm = point.x;
    position.y_mm = point.y;
    position.angle_radian = orientation_radian;

    position_consigne=position;
    Asser_Position(position);

}

struct position_t Trajet_get_consigne(){
    return position_consigne;
}

/// @brief Calcule la vitesse à partir de l'accelération du robot, de la vitesse maximale et de la contrainte en fin de trajectoire
/// @param pas_de_temps_s : temps écoulé en ms
/// @return vitesse déterminée en m/s
double Trajet_calcul_vitesse(double pas_de_temps_s){
    double vitesse_max_contrainte;
    double distance_contrainte;
    double vitesse;
    // Calcul de la vitesse avec acceleration
    vitesse = vitesse_mm_s + acceleration_mm_ss * pas_de_temps_s;

    // Calcul de la vitesse maximale due à la contrainte en fin de trajectoire (0 mm/s)
    // https://poivron-robotique.fr/Consigne-de-vitesse.html
    distance_contrainte = Trajectoire_get_longueur_mm(&trajet_trajectoire) - position_mm;
    // En cas de dépassement, on veut garder la contrainte, pour l'instant
    if(distance_contrainte > 0){
        vitesse_max_contrainte = sqrt(2 * acceleration_mm_ss * distance_contrainte);
    }else{
        vitesse_max_contrainte = 0;
    }

    // Selection de la vitesse la plus faible
    if(vitesse > vitesse_max_contrainte){
        vitesse = vitesse_max_contrainte;
    }
    if(vitesse > VITESSE_MAX_MM_S){
        vitesse = VITESSE_MAX_MM_S;
    }
    return vitesse;
}
