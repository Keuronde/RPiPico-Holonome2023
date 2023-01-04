#include "pico/stdlib.h"
#include "hardware/i2c.h"

enum i2c_resultat_t {
    I2C_EN_COURS,
    I2C_SUCCES,
    I2C_ECHEC
};

void i2c_maitre_init(void);
void i2c_gestion(i2c_inst_t *i2c);
enum i2c_resultat_t i2c_lire_registre_nb(uint8_t adresse_7_bits, uint8_t registre, uint8_t * reception, uint8_t len);
int i2c_ecrire_registre(char adresse_7_bits, char registre, char valeur_registre);
int i2c_lire_registre(char adresse_7_bits, char registre, char * reception, char len);
