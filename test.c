#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "gyro.h"
#include "Temps.h"
#include "spi_nb.h"
#include "Servomoteur.h"

const uint LED_PIN = 25;

#define V_INIT -999.0

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    double vitesse_filtre_x=V_INIT, vitesse_filtre_y=V_INIT, vitesse_filtre_z=V_INIT;
    struct t_angle_gyro_double angle_gyro;

    uint32_t temps_ms = 0, temps_ms_old;

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    sleep_ms(3000);
    Servomoteur_Init();
    //puts("Debut");
    //spi_test();

    //while(1);
    Temps_init();
    Gyro_Init();
    temps_ms = Temps_get_temps_ms();
    temps_ms_old = temps_ms;
    while (1) { 
        u_int16_t step_ms = 5;
        float coef_filtre = 0.001;
        
        while(temps_ms_old == Temps_get_temps_ms());
        temps_ms_old = Temps_get_temps_ms();
        
        // Tous les pas de step_ms
        if(Temps_get_temps_ms() % step_ms){
            Gyro_Read(step_ms);
            //gyro_affiche(gyro_get_vitesse(), "Angle :");
            // Filtre 
            angle_gyro = gyro_get_vitesse();
            if(vitesse_filtre_x == V_INIT){
                vitesse_filtre_x = angle_gyro.rot_x;
                vitesse_filtre_y = angle_gyro.rot_y;
                vitesse_filtre_z = angle_gyro.rot_z;
            }else{
                vitesse_filtre_x = vitesse_filtre_x * (1-coef_filtre) + angle_gyro.rot_x * coef_filtre;
                vitesse_filtre_y = vitesse_filtre_y * (1-coef_filtre) + angle_gyro.rot_y * coef_filtre;
                vitesse_filtre_z = vitesse_filtre_z * (1-coef_filtre) + angle_gyro.rot_z * coef_filtre;
            }

            printf("%f, %f, %f, %f\n", (double)temps_ms_old / 1000, vitesse_filtre_x, vitesse_filtre_y, vitesse_filtre_z);
            //gyro_affiche(angle_gyro, "Vitesse (°/s),");
        }

        // Toutes les 50 ms
        if((Temps_get_temps_ms() % 50) == 0){
            
        }

        // Toutes les 500 ms
        if((Temps_get_temps_ms() % 500) == 0){
            //gyro_affiche(gyro_get_angle(), "Angle :");
        }
        // Toutes les secondes
        if((Temps_get_temps_ms() % 500) == 0){
            //gyro_get_temp();
        }
    }
}
