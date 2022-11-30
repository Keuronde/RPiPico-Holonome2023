#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "QEI.h"
#include "quadrature_encoder.pio.h"


/*** C'est ici que se fait la conversion en mm 
 * ***/

// Roues 60 mm de diamètre, 188,5 mm de circonférence
// Réduction Moteur 30:1
// Réduction poulie 16:12
// Nombre d'impulsions par tour moteur : 200
// Nombre d'impulsions par tour réducteur : 6000
// Nombre d'impulsions par tour de roue : 8000
// Impulsion / mm : 42,44

#define IMPULSION_PAR_MM (42.44f)
#define ASSERMOTEUR_GAIN_P 160
#define ASSERMOTEUR_GAIN_I .80f

struct QEI_t QEI_A, QEI_B, QEI_C;

PIO pio_QEI = pio0;

const uint CODEUR_1_A = 26;
const uint CODEUR_1_B = 27;

void QEI_init(){
    // Initialisation des 3 modules QEI
    // Chaque module QEI sera dans une machine à état du PIO 0
    

    // Offset le début du programme
    // Si ce n'est pas 0, le programme ne marchera pas
    uint offset = pio_add_program(pio_QEI, &quadrature_encoder_program); 
    if(offset != 0){
        printf("PIO init error: offset != 0");
    }
    // bizarrement, il faut initialiser les boches en entrée pour les GPIO 26 et 27.
    // Probablement car elle sont en analogique par défaut...
    gpio_init(CODEUR_1_A);
    gpio_set_dir(CODEUR_1_A, GPIO_IN);

    gpio_init(CODEUR_1_B);
    gpio_set_dir(CODEUR_1_B, GPIO_IN);

    // Initialisation des "machines à états" :
    // QEI1 : broche 31 et 32 - pio : pio0, sm : 0, Offset : 0, broches (GPIO) 26 et 27, clock div : 0 pour commencer
    // QEI1 : !!! Attention, il faudra modifier la carte élec !!!
    quadrature_encoder_program_init(pio_QEI, 0, offset, 26, 0);
    // QEI2 : broche 26 et 27 - pio : pio0, sm : 1, Offset : 0, broches (GPIO) 20 et 21, clock div : 0 pour commencer
    quadrature_encoder_program_init(pio_QEI, 1, offset, 20, 0);
    // QEI3 : broche 24 et 25 - pio : pio0, sm : 1, Offset : 0, broches (GPIO) 18 et 19, clock div : 0 pour commencer
    quadrature_encoder_program_init(pio_QEI, 2, offset, 18, 0);

    QEI_A.value=0;
    QEI_B.value=0;
    QEI_C.value=0;

}

/// @brief Lit les modules QEI et stock l'écart en cette lecture et la lecture précédente.
void QEI_update(void){
    
    int old_value;

    old_value = QEI_A.value;
    QEI_A.value = quadrature_encoder_get_count(pio_QEI, 0);
    QEI_A.delta = QEI_A.value - old_value;

    old_value = QEI_B.value;
    QEI_B.value = quadrature_encoder_get_count(pio_QEI, 1);
    QEI_B.delta = QEI_B.value - old_value;

    old_value = QEI_C.value;
    QEI_C.value = quadrature_encoder_get_count(pio_QEI, 2);
    QEI_C.delta = QEI_C.value - old_value;

}

/// @brief Renvoi le nombre d'impulsion du module QEI depuis la lecture précédente
/// Les signe sont inversés (sauf A) car le reducteur inverse le sens de rotation.
/// Attention, le signe du QEI_A est inversé par rapport aux autres à cause d'un soucis sur la carte électornique
/// @param  qei : Nom du module à lire (QEI_A_NAME, QEI_B_NAME ou QEI_C_NAME)
/// @return Nombre d'impulsion calculé lors du dernier appel de la function QEI_Update()
int QEI_get(enum QEI_name_t qei){
    switch (qei)
    {
    case QEI_A_NAME:
        return QEI_A.delta;
        break;

    case QEI_B_NAME:
        return -QEI_B.delta;
        break;
    
    case QEI_C_NAME:
        return -QEI_C.delta;
        break;
    
    default:
        break;
    }
}

/// @brief Renvoi la distance parcourue en mm depuis la lecture précédente
/// @param  qei : Nom du module à lire (QEI_A_NAME, QEI_B_NAME ou QEI_C_NAME)
/// @return la distance parcourue en mm calculée lors du dernier appel de la function QEI_Update()
double QEI_get_mm(enum QEI_name_t qei){
    return (double) QEI_get(qei) / (double)IMPULSION_PAR_MM;
}