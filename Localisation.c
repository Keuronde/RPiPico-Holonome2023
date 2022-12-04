#include "Localisation.h"
#include "QEI.h"
#include "math.h"
#include "Geometrie_robot.h"

struct position_t position;

void Localisation_init(){
    position.x_mm = 0;
    position.y_mm = 0;
    position.angle_radian = 0;
}

void Localisation_gestion(){
    // Voir http://poivron-robotique.fr/Robot-holonome-localisation-partie-2.html
    double distance_roue_a_mm = QEI_get_mm(QEI_A_NAME);
    double distance_roue_b_mm = QEI_get_mm(QEI_B_NAME);
    double  distance_roue_c_mm = QEI_get_mm(QEI_C_NAME);
    double delta_x_ref_robot, delta_y_ref_robot;

    delta_x_ref_robot = (distance_roue_a_mm + distance_roue_b_mm - 2 * distance_roue_c_mm)  / 3.0;
    delta_y_ref_robot = (-distance_roue_a_mm + distance_roue_b_mm)  * RACINE_DE_3 / 3.0;

    position.angle_radian += - ( distance_roue_a_mm + distance_roue_b_mm + distance_roue_c_mm) / (3 * DISTANCE_ROUES_CENTRE_MM);

    // Projection dans le référentiel du robot
    position.x_mm += delta_x_ref_robot * cos(position.angle_radian) - delta_y_ref_robot * sin(position.angle_radian);
    position.y_mm += delta_x_ref_robot * sin(position.angle_radian) + delta_y_ref_robot * cos(position.angle_radian);

}

struct position_t Localisation_get(void){
    return position;
}
