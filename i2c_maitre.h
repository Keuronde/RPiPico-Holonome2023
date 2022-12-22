#define I2C_EN_COURS 0
#define I2C_SUCCES 1
#define I2C_ECHEC -1



void i2c_maitre_init(void);
int i2c_ecrire_registre(char adresse_7_bits, char registre, char valeur_registre);
int i2c_lire_registre(char adresse_7_bits, char registre, char * reception, char len);