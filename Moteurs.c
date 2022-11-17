#include "hardware/pwm.h"
#include "Moteurs.h"

#define MOTEUR_A 0
#define MOTEUR_B 1
#define MOTEUR_C 2

#define MOTEUR_A_VITESSE 6
#define MOTEUR_B_VITESSE 8
#define MOTEUR_C_VITESSE 10

#define MOTEUR_A_SENS 5
#define MOTEUR_B_SENS 7
#define MOTEUR_C_SENS 9

uint slice_moteur_A, slice_moteur_B, slice_moteur_C;


void Moteur_Init(){
    // Initialisation des broches pour les PWM
    gpio_set_function(MOTEUR_A_VITESSE, GPIO_FUNC_PWM);
    gpio_set_function(MOTEUR_B_VITESSE, GPIO_FUNC_PWM);
    gpio_set_function(MOTEUR_C_VITESSE, GPIO_FUNC_PWM);

    // Initialisation des broches pour les sens
    gpio_init(MOTEUR_A_SENS);
    gpio_init(MOTEUR_B_SENS);
    gpio_init(MOTEUR_C_SENS);
    gpio_set_dir(MOTEUR_A_SENS, true);
    gpio_set_dir(MOTEUR_B_SENS, true);
    gpio_set_dir(MOTEUR_C_SENS, true);

    // Configuration des PWM à 1 kHz
    // Clock_system 125 MHz - 16 bits PWM (65536)
    // Donc si on veut profiter de toute la résolution du PWM,
    // il faut une fréquence d'entrée à 65,536 MHz
    // Soit un diviseur de 2 (apprximativement)

    slice_moteur_A = pwm_gpio_to_slice_num(MOTEUR_A_VITESSE); // GPIO6  -> PWM 3A (voir RP2040 Datasheet 4.5.2)
    slice_moteur_B = pwm_gpio_to_slice_num(MOTEUR_B_VITESSE); // GPIO8  -> PWM 4A
    slice_moteur_C = pwm_gpio_to_slice_num(MOTEUR_C_VITESSE); // GPIO10 -> PWM 5A

    pwm_config pwm_moteur = pwm_get_default_config();

    pwm_config_set_clkdiv_int(&pwm_moteur, 2); 
    pwm_config_set_phase_correct(&pwm_moteur, false);
    pwm_config_set_wrap(&pwm_moteur, 65635);

    pwm_init(slice_moteur_A, &pwm_moteur, true);
    pwm_init(slice_moteur_B, &pwm_moteur, true);
    pwm_init(slice_moteur_C, &pwm_moteur, true);

    Moteur_set_vitesse(MOTEUR_A, 0);
    Moteur_set_vitesse(MOTEUR_B, 0);
    Moteur_set_vitesse(MOTEUR_C, 0);

}


Moteur_set_vitesse(enum t_moteur moteur, int16 vitesse){
    uint16 u_vitesse;

    // Le PWM accepte 16 bits de résolution, on se remet sur 16 bits (et non sur 15 + signe)
    if (vitesse < 0){
        u_vitesse = -vitesse;
    }
    u_vitesse = vitesse * 2;

    switch(moteur){
        case MOTEUR_A:
            pwm_set_chan_level(slice_moteur_A, PWM_CHAN_A, u_vitesse);
            if(vitesse < 0){
                gpio_put(MOTEUR_A_SENS, false);
            }else{
                gpio_put(MOTEUR_A_SENS, true);
            }
            break;

        case MOTEUR_B:
            pwm_set_chan_level(slice_moteur_B, PWM_CHAN_A, u_vitesse);
            if(vitesse < 0){
                gpio_put(MOTEUR_B_SENS, false);
            }else{
                gpio_put(MOTEUR_B_SENS, true);
            }
            break;

        case MOTEUR_C:
            pwm_set_chan_level(slice_moteur_C, PWM_CHAN_A, u_vitesse);
            if(vitesse < 0){
                gpio_put(MOTEUR_B_SENS, false);
            }else{
                gpio_put(MOTEUR_B_SENS, true);
            }
            break;
    }

}