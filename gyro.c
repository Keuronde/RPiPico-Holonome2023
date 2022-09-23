#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

const uint PIN_CS = 1;

static inline void cs_select();
static inline void cs_deselect();
int gyro_init_check();
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

    spi_init(spi0, 100 * 1000); // SPI init @ 100 kHZ

    //Ça doit être les valeurs par défaut, mais ça marche !
    spi_set_format(spi0, 8, SPI_CPHA_1, SPI_CPOL_1, SPI_MSB_FIRST);

    // Test de la présence du gyroscope :
    if(gyro_init_check()){
        puts("Gyroscope non trouve");
    }else{
        puts("Gyroscope trouve");
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


int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire){
    uint8_t reg = registrer | 0xC0 ;
    int nb_recu;
    cs_select();
    spi_write_blocking(spi0, &reg, 1);
    sleep_ms(10);
    nb_recu = spi_read_blocking(spi0, 0, tampon, nb_a_lire);
    cs_deselect();
    
}

void Gyro_Read(void){
    uint8_t tampon[10]="";
    uint8_t reg[2] = {0x0F | 0x80 | 0x40, '\0'};
    int nb_recu;
    // Lire l'adresse d'identification
    // WHO_AM_I : 0x0F
    cs_select();
    puts("Envoi");

    spi_write_blocking(spi0, reg, 1);
    // Doit répondre : 0b1101 0111
    puts(reg);
    puts("Lecture");
    sleep_ms(10);

    nb_recu = spi_read_blocking(spi0, 0x55, tampon, 1);
    tampon[nb_recu]='\0'; 
    puts(tampon);
    cs_deselect();
    
    


}
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}
