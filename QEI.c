#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "QEI.h"
#include "quadrature_encoder.pio.h"


struct QEI_t QEI_A, QEI_B, QEI_C;

PIO pio_QEI = pio0;

void QEI_init(){
    // Initialisation des 3 modules QEI
    // Chaque module QEI sera dans une machine à état du PIO 0
    

    // Offset le début du programme
    // Si ce n'est pas 0, le programme ne marchera pas
    uint offset = pio_add_program(pio_QEI, &quadrature_encoder_program); 
    if(offset != 0){
        printf("PIO init error: offset != 0");
    }

    // Initialisation des "machines à états" :
    // QEI1 : broche 26 et 27 - pio : pio0, sm : 0, Offset : 0, broches 26 et 27, clock div : 0 pour commencer
    // QEI1 : !!! Attention, il faudra modifier la carte élec !!!
    quadrature_encoder_program_init(pio_QEI, 0, offset, 26, 0);
    // QEI2: broche 26 et 27 - pio : pio0, sm : 1, Offset : 0, broches 26 et 27, clock div : 0 pour commencer
    quadrature_encoder_program_init(pio_QEI, 1, offset, 20, 0);
    // QEI3: broche 24 et 25 - pio : pio0, sm : 1, Offset : 0, broches 26 et 27, clock div : 0 pour commencer
    quadrature_encoder_program_init(pio_QEI, 2, offset, 24, 0);

    QEI_A.value=0;
    QEI_B.value=0;
    QEI_C.value=0;

}

void QEI_update(){
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

int QEI_get(enum t_moteur moteur){
    switch (moteur)
    {
    case MOTEUR_A:
        return QEI_A.value;
        break;

    case MOTEUR_B:
        return QEI_B.value;
        break;
    
    case MOTEUR_C:
        return QEI_C.value;
        break;
    
    default:
        break;
    }
}