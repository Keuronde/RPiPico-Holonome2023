#include "gyro_L3GD20H.h"
#include "spi_nb.h"
#include <stdio.h>

#ifdef GYRO_L3GD20H

int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire){
    uint8_t reg = registrer | 0xC0 ;
    int nb_recu;
    cs_select();
    spi_write_blocking(spi0, &reg, 1);
    sleep_ms(10);
    nb_recu = spi_read_blocking(spi0, 0, tampon, nb_a_lire);
    cs_deselect();
    
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


int gyro_config(){
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
        //puts("gyro_config ok !");
        return 0;
    }else{
        //printf("gyro_config FAILED ! :%#4x\n", tampon2[1]);
        return 1;
    }
    // Registre 


}


void gyro_get_vitesse_brute(struct t_angle_gyro* angle_gyro, struct t_angle_gyro* angle_gyro_moy){
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

void gyro_get_vitesse_normalisee(struct t_angle_gyro* _vitesse_angulaire,
        struct t_angle_gyro_double * _vitesse_gyro){
    _vitesse_gyro->rot_x = (double)_vitesse_angulaire->rot_x * 0.00875 / 32.0;
    _vitesse_gyro->rot_y = (double)_vitesse_angulaire->rot_y * 0.00875 / 32.0;
    _vitesse_gyro->rot_z = (double)_vitesse_angulaire->rot_z * 0.00875 / 32.0;
}

#endif