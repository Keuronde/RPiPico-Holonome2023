#include "pico/stdlib.h"

bool Temps_increment(struct repeating_timer *t);
void Temps_init(void);
uint32_t Temps_get_temps_ms(void);