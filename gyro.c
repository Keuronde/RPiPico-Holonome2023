#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/structs/spi.h"
#include "spi_nb.h"
#include "Temps.h"
#include "gyro.h"

struct t_angle_gyro{
    int32_t rot_x, rot_y, rot_z, temp;
};

/// @brief structure d'échange des angles du gyrocope
struct t_angle_gyro vitesse_angulaire, vitesse_calibration;

int gyro_init_check();
void gyro_config();
int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire);
void gyro_get_angles(struct t_angle_gyro* angle_gyro, struct t_angle_gyro* angle_gyro_moy);
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

    vitesse_calibration.rot_x = 0;
    vitesse_calibration.rot_y = 0;
    vitesse_calibration.rot_z = 0;
    vitesse_calibration.temp = 0;

    //spi_init(spi0, 100 * 1000); // SPI init @ 100 kHz
    uint speed = spi_init(spi0, 2 * 1000 * 1000); // SPI init @ 2 MHz
    printf("vitesse SPI : %d\n", speed);

    //Ça doit être les valeurs par défaut, mais ça marche !
    spi_set_format(spi0, 8, SPI_CPHA_1, SPI_CPOL_1, SPI_MSB_FIRST);

    // Test de la présence du gyroscope :
    if(gyro_init_check()){
        puts("Gyroscope non trouve");
    }else{
        puts("Gyroscope trouve");
        gyro_config();        
    }
    sleep_ms(150); // Temps d'init du gyroscope
    /*while(1){
        gyro_calibration();
    }*/
}

int gyro_init_check(){
    // Renvoi 0 si l'initialisation s'est bien passée
    // Renvoi 1 si le gyroscope n'a pas répondu
    uint8_t tampon[2]="";
    gyro_read_register_blocking(0x0F, tampon, 1);
    if(tampon[0] == 0xd7){
        return 0;
    }
    return 1;
}

void gyro_config(){
    // Registre CTRL1
    // DR : 11
    // BW : 10
    // PD : 1
    // Zen : 1
    // Yen : 1
    // Xen : 1

    uint8_t config = 0b11101111;
    uint16_t tampon[2] = {0x20, config};
    uint8_t tampon2[10]="\0\0\0\0\0\0\0\0\0";
    int statu, nb_read;

    //while(spi_nb_busy(spi0) == SPI_BUSY);
    cs_select();
    int rep = spi_nb_write_data(spi0, tampon, 2);
    if(rep == SPI_ERR_TRANSMIT_FIFO_FULL){
        printf("Erreur: spi_read_register: SPI_ERR_TRANSMIT_FIFO_FULL\n");
        //return statu;
    }
    while(spi_nb_busy(spi0));
    cs_deselect();

    int nb_lu = spi_read_register(spi0, 0x20, tampon2, 1);
    


    printf("Nb lu: %d\n", nb_lu);
    
    if(tampon2[1] == config){
        puts("gyro_config ok !");
    }else{
        printf("gyro_config FAILED ! :%#4x\n", tampon2[1]);
    }
    // Registre 


}


int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire){
    uint8_t reg = registrer | 0xC0 ;
    int nb_recu;
    cs_select();
    spi_write_blocking(spi0, &reg, 1);
    sleep_ms(10);
    nb_recu = spi_read_blocking(spi0, 0, tampon, nb_a_lire);
    cs_deselect();
    
}

void Gyro_Read(uint16_t step_ms){
    uint8_t tampon[10]="\0\0\0\0\0\0\0\0\0";
    uint8_t tampon2[10]="ABCDEFGHI";
    int16_t rot_x, rot_y, rot_z;
    static double angle_x=0, angle_y=0, angle_z=0;
    int nb_recu;


    //spi_read_register(spi0, 0x20, tampon, 1);
    //printf ("Gyro CTRL1 (bis) : %#4x\n", tampon[1] );

    //printf ("RPI SSPCPSR : %#4x\n", spi_get_hw(spi0)->cpsr );
    //printf ("RPI SSPCR0 : %#4x\n", spi_get_hw(spi0)->cr0 );

    gyro_get_angles(&vitesse_angulaire, &vitesse_calibration);
    //gyro_get_angles(&vitesse_angulaire, NULL);

    // Angle en degré
    vitesse_gyro.rot_x = (double)vitesse_angulaire.rot_x * 0.00875 / 32.0;
    vitesse_gyro.rot_y = (double)vitesse_angulaire.rot_y * 0.00875 / 32.0;
    vitesse_gyro.rot_z = (double)vitesse_angulaire.rot_z * 0.00875 / 32.0;

    angle_gyro.rot_x = angle_gyro.rot_x + vitesse_gyro.rot_x * step_ms * 0.001;
    angle_gyro.rot_y = angle_gyro.rot_y + vitesse_gyro.rot_y * step_ms * 0.001;
    angle_gyro.rot_z = angle_gyro.rot_z + vitesse_gyro.rot_z * step_ms * 0.001;

    //printf("%d, %#4x, %#4x, %#4x\n", step_ms, vitesse_angulaire.rot_x, vitesse_angulaire.rot_y, vitesse_angulaire.rot_z);
    
    //printf("rx : %f, ry : %f, rz: %f\n", angle_gyro.rot_x, angle_gyro.rot_y, angle_gyro.rot_z);

    //while(spi_nb_busy(spi0));
    //spi_nb_read_data_8bits(spi0,tampon);
    //printf("tampon : %s\n", tampon);
}

int16_t gyro_get_temp(void){
    int8_t tampon[3]="\0\0";
    int16_t temperature;
    spi_read_register(spi0, 0x26, tampon, 6);
    temperature = -tampon[1];
    printf("temperature %d\n",temperature);

}

void gyro_get_angles(struct t_angle_gyro* angle_gyro, struct t_angle_gyro* angle_gyro_moy){
    uint8_t tampon[10]="\0\0\0\0\0\0\0\0\0";
    int16_t rot_x, rot_y, rot_z;
    spi_read_register(spi0, 0x28, tampon, 6);
    
    rot_x = -(tampon[1] + (tampon[2] << 8));
    rot_y = -(tampon[3] + (tampon[4] << 8));
    rot_z = -(tampon[5] + (tampon[6] << 8));

    if(angle_gyro_moy == NULL){
        angle_gyro->rot_x = (int32_t) rot_x * 32;
        angle_gyro->rot_y = (int32_t) rot_y * 32;
        angle_gyro->rot_z = (int32_t) rot_z * 32;
    }else{
        angle_gyro->rot_x = (int32_t) rot_x * 32 - angle_gyro_moy->rot_x;
        angle_gyro->rot_y = (int32_t) rot_y * 32 - angle_gyro_moy->rot_y;
        angle_gyro->rot_z = (int32_t) rot_z * 32 - angle_gyro_moy->rot_z;
    }
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

    printf("Calibration...\n");

    vitesse_calibration.rot_x = 0;
    vitesse_calibration.rot_y = 0;
    vitesse_calibration.rot_z = 0;
    
    // Acquisition des échantillons, 1 par milliseconde (1 ms, c'est trop court on dirait !)
    for(uint32_t i=0; i<nb_ech; i++){
        while(m_temps_ms == Temps_get_temps_ms());
        m_temps_ms = Temps_get_temps_ms();
        gyro_get_angles(&vitesse_angulaire, NULL);
        vitesse_calibration.rot_x += vitesse_angulaire.rot_x;
        vitesse_calibration.rot_y += vitesse_angulaire.rot_y;
        vitesse_calibration.rot_z += vitesse_angulaire.rot_z;
        if(m_temps_ms > temps_500ms){
            printf(".");
            gyro_get_temp();
            temps_500ms += 500;
        }
        sleep_ms(5);
    }
    vitesse_calibration.rot_x = vitesse_calibration.rot_x / (int32_t)nb_ech;
    vitesse_calibration.rot_y = vitesse_calibration.rot_y / (int32_t)nb_ech;
    vitesse_calibration.rot_z = vitesse_calibration.rot_z / (int32_t)nb_ech;
    temperature = gyro_get_temp();

    printf("fin calibration, %d, %d, %d, %d\n", vitesse_calibration.rot_x, vitesse_calibration.rot_y ,vitesse_calibration.rot_z, temperature);

}


