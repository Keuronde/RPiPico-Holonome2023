#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/structs/spi.h"
#include "spi_nb.h"

const uint PIN_CS = 1;

int gyro_init_check();
void gyro_config();
int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire);


void Gyro_Init(void){
    // 
    gpio_set_function(16, GPIO_FUNC_SPI); // SDI
    gpio_set_function(18, GPIO_FUNC_SPI); // SCK
    gpio_set_function(19, GPIO_FUNC_SPI); // SDO
    gpio_set_function(PIN_CS, GPIO_OUT); // CSn

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    cs_deselect();

    spi_init(spi0, 100 * 1000); // SPI init @ 100 kHz
    //spi_init(spi0, 2 * 1000 * 1000); // SPI init @ 2 MHz

    //Ça doit être les valeurs par défaut, mais ça marche !
    spi_set_format(spi0, 8, SPI_CPHA_1, SPI_CPOL_1, SPI_MSB_FIRST);

    // Test de la présence du gyroscope :
    if(gyro_init_check()){
        puts("Gyroscope non trouve");
    }else{
        puts("Gyroscope trouve");
        gyro_config();
    }
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

    uint16_t tampon[2] = {0x20, 0b11101111};
    while(spi_nb_busy(spi0) == SPI_BUSY);
    cs_select();
    int rep = spi_nb_write_data(spi0, tampon, 2);
    while(spi_nb_busy(spi0) == SPI_BUSY);
    cs_deselect();

    if (rep == SPI_ERR_TRANSMIT_FIFO_FULL){
        while(1){
            printf("gyro_config: SPI_ERR_TRANSMIT_FIFO_FULL\n");
        }
    }else{
        puts("gyro_config ok !");
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
    uint8_t tampon[10]="123456789";
    int16_t rot_x, rot_y, rot_z;
    static double angle_x=0, angle_y=0, angle_z=0;
    int nb_recu;


    //cs_select();
    //while(spi_nb_read_register_8bits(spi0, 0x20, tampon, 1) == SPI_IN_PROGRESS);
    //cs_deselect();
    gyro_read_register_blocking(0x20, tampon, 1);
    //printf ("Gyro CTRL1 : %#4x\n", tampon[0] );


    spi_read_register(spi0, 0x20, tampon, 1);
    //printf ("Gyro CTRL1 (bis) : %#4x\n", tampon[1] );

    //printf ("RPI SSPCPSR : %#4x\n", spi_get_hw(spi0)->cpsr );
    //printf ("RPI SSPCR0 : %#4x\n", spi_get_hw(spi0)->cr0 );


    //gyro_read_register_blocking(0x28, tampon, 6);
    spi_read_register(spi0, 0x28, tampon, 6);

    for(int i=0; i<10; i++){
        printf("%#4x ", tampon[i]);
    }

    rot_x = -(tampon[1] + (tampon[2] << 8));
    rot_y = -(tampon[3] + (tampon[4] << 8));
    rot_z = -(tampon[5] + (tampon[6] << 8));

    angle_x = angle_x + (double)rot_x * step_ms * 0.001 * 0.00875;
    angle_y = angle_y + (double)rot_y * step_ms * 0.001 * 0.00875;
    angle_z = angle_z + (double)rot_z * step_ms * 0.001 * 0.00875;
    
    printf("rx : %f, ry : %f, rz: %f\n", angle_x, angle_y, angle_z);


}