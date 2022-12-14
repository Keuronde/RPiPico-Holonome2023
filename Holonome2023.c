#include <stdio.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/binary_info.h"
#include "math.h"
#include "Test.h"

#include "gyro.h"
#include "Asser_Moteurs.h"
#include "Asser_Position.h"
#include "Commande_vitesse.h"
#include "Localisation.h"
#include "Moteurs.h"
#include "QEI.h"
#include "Servomoteur.h"
#include "spi_nb.h"
#include "Temps.h"
#include "Trajectoire.h"
#include "Trajet.h"

const uint LED_PIN = 25;
const uint LED_PIN_ROUGE = 28;
const uint LED_PIN_NE_PAS_UTILISER = 22;

uint temps_cycle;


#define V_INIT -999.0
#define TEST_TIMEOUT_US 10000000

int mode_test();

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    double vitesse_filtre_x=V_INIT, vitesse_filtre_y=V_INIT, vitesse_filtre_z=V_INIT;
    struct t_angle_gyro_double angle_gyro;

    uint32_t temps_ms = 0, temps_ms_old;
    uint32_t temps_us_debut_cycle;

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    gpio_init(LED_PIN_ROUGE);
    gpio_set_dir(LED_PIN_ROUGE, GPIO_OUT);
    gpio_put(LED_PIN_ROUGE, 1);

    // Il faut neutraliser cette broche qui pourrait interférer avec 
    // la lecture des codeurs. (problème sur la carte électrique)...
    gpio_init(LED_PIN_NE_PAS_UTILISER);
    gpio_set_dir(LED_PIN_NE_PAS_UTILISER, GPIO_IN);

    sleep_ms(3000);
    Servomoteur_Init();
    //puts("Debut");
    //spi_test();

    //while(1);
    Temps_init();
    Moteur_Init();
    QEI_init();
    AsserMoteur_Init();
    Localisation_init();

    while(mode_test());

    Gyro_Init();

    

    temps_ms = Temps_get_temps_ms();
    temps_ms_old = temps_ms;
    while (1) { 
        u_int16_t step_ms = 2;
        float coef_filtre = 1-0.8;
        
        while(temps_ms == Temps_get_temps_ms());
        temps_ms = Temps_get_temps_ms();
        temps_ms_old = temps_ms;
        
        // Tous les pas de step_ms
        if(!(temps_ms % step_ms)){
            temps_us_debut_cycle = time_us_32();
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
            temps_cycle = time_us_32() - temps_us_debut_cycle;

            //printf("%#x, %#x\n", (double)temps_ms_old / 1000,  vitesse_filtre_z);

            //printf("%d, %d\n", temps_ms, (int32_t) (vitesse_filtre_z * 1000));
            //gyro_affiche(angle_gyro, "Vitesse (°/s),");
            
        }

        // Toutes les 50 ms
        if((Temps_get_temps_ms() % 50) == 0){
            struct t_angle_gyro_double m_gyro;
            m_gyro = gyro_get_angle_degres();
            printf("%f, %f, %d\n", (double)temps_ms / 1000,  m_gyro.rot_z, temps_cycle);
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
