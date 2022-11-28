#include "Moteurs.h"

void AsserMoteur_setConsigne_mm_s(enum t_moteur moteur, double consigne_mm_s);
double AsserMoteur_getVitesse_mm_s(enum t_moteur moteur, int step_ms);
void AsserMoteur_Gestion(int step_ms);
void AsserMoteur_Init();