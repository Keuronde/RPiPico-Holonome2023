#include "Moteurs.h"

struct QEI_t{
    int value;
    int delta;
};

extern struct QEI_t QEI_A, QEI_B, QEI_C;

void QEI_update(void);
void QEI_init(void);
int QEI_get(enum t_moteur moteur);