#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/structs/spi.h"
#include "spi_nb.h"
#include "Temps.h"
#include "gyro.h"

#define GYRO_L3GD20H

#ifdef GYRO_L3GD20H
    #include "gyro_L3GD20H.h"
#else
    #ifdef GYRO_ADXRS453
        #include "gyro_ADXRS453.h"
    #else
        #error "Choissisez un gyroscope"
    #endif
#endif


/// @brief structure d'échange des angles du gyrocope
struct t_angle_gyro _vitesse_calibration;
struct t_angle_gyro *vitesse_calibration;
struct t_angle_gyro_double _vitesse_angulaire;
struct t_angle_gyro_double *vitesse_angulaire;

int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire);
void gyro_calibration(void);

uint32_t rot_x_zero, rot_y_zero, rot_z_zero;



struct t_angle_gyro_double angle_gyro, vitesse_gyro;


struct t_angle_gyro_double gyro_get_angle(void){
    return angle_gyro;
}
struct t_angle_gyro_double gyro_get_vitesse(void){
    return vitesse_gyro;
}

void Gyro_Init(void){
    // 
    gpio_set_function(16, GPIO_FUNC_SPI); // SDI
    gpio_set_function(18, GPIO_FUNC_SPI); // SCK
    gpio_set_function(19, GPIO_FUNC_SPI); // SDO
    gpio_set_function(PIN_CS, GPIO_OUT); // CSn

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    cs_deselect();

    vitesse_calibration = NULL;
    vitesse_angulaire = &_vitesse_angulaire;

    //spi_init(spi0, 100 * 1000); // SPI init @ 100 kHz
    uint speed = spi_init(spi0, 2 * 1000 * 1000); // SPI init @ 2 MHz
    printf("vitesse SPI : %d\n", speed);


    spi_set_format(spi0, 8, SPI_CPHA_1, SPI_CPOL_1, SPI_MSB_FIRST);

    // Test de la présence du gyroscope :
    if(gyro_init_check()){
        puts("Gyroscope non trouve");
    }else{
        puts("Gyroscope trouve");
        if(!gyro_config()){
            puts("gyro_config ok !");
        }else{
            printf("gyro_config FAILED !");

        }
    }
    sleep_ms(150); // Temps d'init du gyroscope
    /*while(1){
        gyro_calibration();
    }*/
}

void Gyro_Read(uint16_t step_ms){
    uint8_t tampon[10]="\0\0\0\0\0\0\0\0\0";
    uint8_t tampon2[10]="ABCDEFGHI";
    int16_t rot_x, rot_y, rot_z;
    static double angle_x=0, angle_y=0, angle_z=0;
    struct t_angle_gyro * _vitesse_angulaire_brute;
    struct t_angle_gyro  m_vitesse_angulaire_brute;
    int nb_recu;

    _vitesse_angulaire_brute = &m_vitesse_angulaire_brute;

    // Acquisition des valeurs
    gyro_get_vitesse_brute(_vitesse_angulaire_brute, vitesse_calibration);
    //gyro_get_angles(&vitesse_angulaire, NULL);


    // conversion de la vitesse angulaire en degré/seconde
    gyro_get_vitesse_normalisee(_vitesse_angulaire_brute, vitesse_angulaire);

    vitesse_gyro = *vitesse_angulaire;

    // Intégration en fonction du pas de temps
    angle_gyro.rot_x = angle_gyro.rot_x + vitesse_angulaire->rot_x * step_ms * 0.001;
    angle_gyro.rot_y = angle_gyro.rot_y + vitesse_angulaire->rot_y * step_ms * 0.001;
    angle_gyro.rot_z = angle_gyro.rot_z + vitesse_angulaire->rot_z * step_ms * 0.001;
}

int16_t gyro_get_temp(void){
    int8_t tampon[3]="\0\0";
    int16_t temperature;
    spi_read_register(spi0, 0x26, tampon, 6);
    temperature = -tampon[1];
    printf("temperature %d\n",temperature);

}

void gyro_affiche(struct t_angle_gyro_double angle_gyro, char * titre){
    if(titre != NULL){
        printf("%s ",titre);
    }
    printf("angle, %f, %f, %f\n", angle_gyro.rot_x, angle_gyro.rot_y, angle_gyro.rot_z);
}

void gyro_calibration(void){
    uint32_t t_calibration_ms = 40000;
    uint32_t nb_ech = t_calibration_ms/5;
    uint32_t m_temps_ms = Temps_get_temps_ms();
    uint32_t temps_500ms = m_temps_ms;
    int16_t temperature;
    struct t_angle_gyro vitesse_grute_gyro;

    printf("Calibration...\n");
    vitesse_calibration = &_vitesse_calibration;

    vitesse_calibration->rot_x = 0;
    vitesse_calibration->rot_y = 0;
    vitesse_calibration->rot_z = 0;
    
    // Acquisition des échantillons, 1 par milliseconde (1 ms, c'est trop court on dirait !)
    for(uint32_t i=0; i<nb_ech; i++){
        while(m_temps_ms == Temps_get_temps_ms());
        m_temps_ms = Temps_get_temps_ms();
        gyro_get_vitesse_brute(&vitesse_grute_gyro, NULL);
        vitesse_calibration->rot_x += vitesse_grute_gyro.rot_x;
        vitesse_calibration->rot_y += vitesse_grute_gyro.rot_y;
        vitesse_calibration->rot_z += vitesse_grute_gyro.rot_z;
        if(m_temps_ms > temps_500ms){
            printf(".");
            gyro_get_temp();
            temps_500ms += 500;
        }
        sleep_ms(5);
    }
    vitesse_calibration->rot_x = vitesse_calibration->rot_x / (int32_t)nb_ech;
    vitesse_calibration->rot_y = vitesse_calibration->rot_y / (int32_t)nb_ech;
    vitesse_calibration->rot_z = vitesse_calibration->rot_z / (int32_t)nb_ech;
    temperature = gyro_get_temp();

    printf("fin calibration, %d, %d, %d, %d\n", vitesse_calibration->rot_x, vitesse_calibration->rot_y ,vitesse_calibration->rot_z, temperature);

}


