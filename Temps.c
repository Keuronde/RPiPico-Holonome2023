#include <stdio.h>
#include "pico/stdlib.h"
#include "Temps.h"

uint32_t temps_ms=0;
struct repeating_timer timer;

bool Temps_increment(struct repeating_timer *t){
    temps_ms++;
    return true;
}

void Temps_init(void){
    temps_ms=0;
    add_repeating_timer_ms(-1, Temps_increment, NULL, &timer);
}

uint32_t Temps_get_temps_ms(void){
    return temps_ms;
}