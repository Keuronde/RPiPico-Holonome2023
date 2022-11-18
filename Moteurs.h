#include "pico/stdlib.h"

enum t_moteur {
    MOTEUR_A=0,
    MOTEUR_B=1,
    MOTEUR_C=2
};

void Moteur_Init(void);
void Moteur_SetVitesse(enum t_moteur moteur, int16_t vitesse );
