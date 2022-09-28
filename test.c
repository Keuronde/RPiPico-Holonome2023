#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "gyro.h"
#include "Temps.h"

const uint LED_PIN = 25;

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    uint32_t temps_ms = 0;

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    sleep_ms(3000);


    Temps_init();
    Gyro_Init();
    temps_ms = Temps_get_temps_ms();
    while (1) { 
        u_int16_t step_ms = 5;
        
        // Tous les pas de step_ms
        if(temps_ms == Temps_get_temps_ms()){
            temps_ms += step_ms;
            Gyro_Read(step_ms);
            
        }

        // Toutes les 500 ms
        if((Temps_get_temps_ms() % 500) == 0){
            gyro_affiche(gyro_get_angle(), "Angle :");
        }
    }
}
