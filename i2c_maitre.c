#include "i2c_maitre.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define I2C_SDA_PIN 16
#define I2C_SCL_PIN 17

void i2c_maitre_init(void){
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);

    printf("Initialisation des broches\n");
    for(int i=0; i++; i<=28){
        if(gpio_get_function(i) == GPIO_FUNC_I2C){
            printf("Borche I2C : %d\n", i);
            gpio_set_function(i, GPIO_FUNC_NULL);
        }
    }

    printf("%d et %d en I2C\n", I2C_SDA_PIN, I2C_SCL_PIN);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
}

/// @brief Pour l'instant bloquant, mais devrait passer en non bloquant bientôt
/// @param adresse_7_bits 
/// @param  
/// @return 0: en cours, 
int i2c_lire_registre(char adresse_7_bits, char registre, char * reception, char len){
    int statu;
    char emission[1];

    emission[0] = registre;
    statu = i2c_write_blocking (i2c0, adresse_7_bits, emission, 1, 0);
    if(statu == PICO_ERROR_GENERIC){
        printf("I2C - Envoi registre Echec\n");
        return I2C_ECHEC;
    }

    statu = i2c_read_blocking (i2c0, adresse_7_bits, reception, len, 0);
    if(statu == PICO_ERROR_GENERIC){
        printf("I2C - Lecture registre Echec\n");
        return I2C_ECHEC;
    }

    return I2C_SUCCES;
}

int i2c_ecrire_registre(char adresse_7_bits, char registre, char valeur_registre){
    int statu;
    char emission[2];

    emission[0] = registre;
    emission[1] = valeur_registre;
    statu = i2c_write_blocking (i2c0, adresse_7_bits, emission, 2, 0);
    if(statu == PICO_ERROR_GENERIC){
        printf("Erreur ecrire registre\n");
        return I2C_ECHEC;
    }

    printf("i2c Registre %x, valeur %x\n", registre, valeur_registre);

    return I2C_SUCCES;
}