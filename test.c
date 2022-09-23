#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"


const uint PIN_CS = 1;
const uint LED_PIN = 25;

void init_gyro(void);
void read_gyro(void);
void read_gyro_i2c();
void init_gyro_i2c();

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

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    init_gyro();

    while (1) { 
        /*gpio_put(LED_PIN, 0);
        sleep_ms(251);
        gpio_put(LED_PIN, 1);
        puts("Bonjour");
        sleep_ms(1000);*/
        sleep_ms(1000);
        read_gyro();
    }
}

void init_gyro_i2c(){
    gpio_set_function(0, GPIO_FUNC_I2C); // SDA
    gpio_set_function(1, GPIO_FUNC_I2C); // SCL
    i2c_init(i2c0, 40 * 1000);
}

void read_gyro_i2c(){
    uint8_t tampon[10]="";
    uint8_t reg = 0x0F;
    int rep;
    uint8_t adresse_7bits = 0b1101011;

    // Envoi adresse registre à lire
    rep = i2c_write_blocking(i2c0, adresse_7bits, &reg, 1, true );
    if(rep == PICO_ERROR_GENERIC){
        puts("Erreur I2C");
    }else{
        puts("envoi OK");
    }

    // Lecture identification 
    rep = i2c_read_blocking(i2c0, adresse_7bits, tampon, 1, false );
    if(rep == PICO_ERROR_GENERIC){
        puts("Erreur I2C");
    }else{
        puts("Reception OK");
        puts(tampon);
    }
}

void init_gyro(void){
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
}
void read_gyro(void){
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