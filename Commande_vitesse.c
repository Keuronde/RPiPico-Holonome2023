#include "Asser_Moteurs.h"
#include "Geometrie_robot.h"

/// @brief Commande de la vitesse dans le référentiel du robot
/// @param vitesse_x_mm_s : Vitesse x en mm/s dans le référentiel du robot
/// @param vitesse_y_mm_s : Vitesse y en mm/s dans le référentiel du robot
/// @param orientation_radian_s : Rotation en radian/s dans le référentiel du robot
void commande_vitesse(double vitesse_x_mm_s, double vitesse_y_mm_s, double orientation_radian_s){
    double vitesse_roue_a, vitesse_roue_b, vitesse_roue_c;

    vitesse_roue_a = vitesse_x_mm_s / 2.0 - vitesse_y_mm_s * RACINE_DE_3 / 2.0 - DISTANCE_ROUES_CENTRE_MM * orientation_radian_s;
    vitesse_roue_b = vitesse_x_mm_s / 2.0 + vitesse_y_mm_s * RACINE_DE_3 / 2.0 - DISTANCE_ROUES_CENTRE_MM * orientation_radian_s;
    vitesse_roue_c = -vitesse_x_mm_s - DISTANCE_ROUES_CENTRE_MM * orientation_radian_s;

    AsserMoteur_setConsigne_mm_s(MOTEUR_A, vitesse_roue_a);
    AsserMoteur_setConsigne_mm_s(MOTEUR_B, vitesse_roue_b);
    AsserMoteur_setConsigne_mm_s(MOTEUR_C, vitesse_roue_c);

}