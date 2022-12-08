#include "Localisation.h"
#include "Commande_vitesse.h"
#include "math.h"

#define GAIN_P_POSITION 100
#define GAIN_P_ORIENTATION 100

/// @brief Asservissement de la position du robot. Les gains sont déterminés pour des positions très proches du robot
/// C'est à la consigne d'être défini avant pour être atteignable.
/// Nécessite l'appel des fonctions QEI_update(); Localisation_gestion(); AsserMoteur_Gestion(_step_ms);
/// @param position_consigne : position à atteindre dans le référentiel de la table.
void Asser_Position(struct position_t position_consigne){
    double vitesse_x_mm_s, vitesse_y_mm_s, rotation_radian_s;
    double vitesse_robot_x_mm_s, vitesse_robot_y_mm_s;
    double delta_x_mm, delta_y_mm, delta_orientation_radian;
    struct position_t position_actuelle;

    position_actuelle = Localisation_get();

    // Calcul de l'erreur
    delta_x_mm = position_consigne.x_mm - position_actuelle.x_mm;
    delta_y_mm = position_consigne.y_mm - position_actuelle.y_mm;
    delta_orientation_radian = position_consigne.angle_radian - position_actuelle.angle_radian;

    // Asservissement
    vitesse_x_mm_s = delta_x_mm * GAIN_P_POSITION;
    vitesse_y_mm_s = delta_y_mm * GAIN_P_POSITION;
    rotation_radian_s = delta_orientation_radian * GAIN_P_ORIENTATION;
    
    // Projection des translations dans le référentiel du robot
    // C'est pas bon, c'est l'inverse !!!
    //vitesse_robot_x_mm_s = cos(position_actuelle.angle_radian) * vitesse_x_mm_s - sin(position_actuelle.angle_radian) * vitesse_y_mm_s;
    //vitesse_robot_y_mm_s = sin(position_actuelle.angle_radian) * vitesse_x_mm_s + cos(position_actuelle.angle_radian) * vitesse_y_mm_s;
    vitesse_robot_x_mm_s = cos(position_actuelle.angle_radian) * vitesse_x_mm_s + sin(position_actuelle.angle_radian) * vitesse_y_mm_s;
    vitesse_robot_y_mm_s = -sin(position_actuelle.angle_radian) * vitesse_x_mm_s + cos(position_actuelle.angle_radian) * vitesse_y_mm_s;

    // Commande en vitesse
    commande_vitesse(vitesse_robot_x_mm_s, vitesse_robot_y_mm_s, rotation_radian_s);
    

}