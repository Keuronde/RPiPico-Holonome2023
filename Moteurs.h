#include "pico/stdlib.h"

enum t_moteur {
    MOTEUR_A,
    MOTEUR_B,
    MOTEUR_C
};

void Moteur_Init(void);
void Moteur_SetVitesse(t_moteur moteur, int16 vitesse );