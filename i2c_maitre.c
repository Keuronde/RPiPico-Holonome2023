#include "i2c_maitre.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define I2C_SDA_PIN 16
#define I2C_SCL_PIN 17

#define I2C_NB_MAX_TAMPON 20

enum i2c_statu_t{
    I2C_STATU_LIBRE,
    I2C_STATU_OCCUPE
} i2c_statu_i2c0;

uint16_t I2C_tampon_envoi[I2C_NB_MAX_TAMPON];
uint8_t I2C_tampon_reception[I2C_NB_MAX_TAMPON];
uint16_t I2C_nb_a_envoyer, I2C_nb_a_recevoir;
uint8_t adresse_7_bits;
uint32_t i2c_error_code; // value of i2c->hw->tx_abrt_source if anything wrong happen, 0 if everything was fine.

enum transaction_statu_t{
    TRANSACTION_EN_COURS,
    TRANSACTION_TERMINEE
} statu_emission, statu_reception;

void i2d_set_adresse_esclave(uint8_t _adresse_7bits);
void i2c_charger_tampon_envoi(uint8_t* emission, uint16_t nb_envoi, uint16_t nb_reception);
enum i2c_resultat_t i2c_transmission(uint8_t _adresse_7bits, uint8_t* emission, uint16_t nb_envoi, uint16_t nb_reception);

void i2c_maitre_init(void){
    //stdio_init_all();
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

    i2c_statu_i2c0 = I2C_STATU_LIBRE;
}

/// @brief Fonction à appeler régulièrement ou en interruption.
/// @param i2c 
void i2c_gestion(i2c_inst_t *i2c){
    // on veut gérer l'i2c avec cette fonction.
    // 2 cas :
    // - Soit écriture simple (plusieurs octets (W))
    // - Soit écriture + lecture (Adresse (W), registre (W), données (R))
    // Pour écrire 1 octet, i2c->hw->data_cmd = xxx, (avec CMD:8 à 0, ) 
    // Pour lire 1 octet, i2c->hw->data_cmd = xxx (avec CMD:8 à 1)
    // Il faut mettre CMD:9 à 1 pour le dernier octet.

    // Envoi des données (ou des demandes de lecture)
    static uint16_t index_envoi=0, index_reception=0;

    // Acquitement des erreurs, pas 100% fonctionnel ! TODO !
    if(i2c->hw->tx_abrt_source !=0){
        i2c_error_code = i2c->hw->tx_abrt_source;
        printf("Erreur I2C tx_abrt_source : %#x\n", i2c_error_code);
        // on efface l'erreur en lisant le registre clr_tx_abrt
        index_envoi = i2c->hw->clr_tx_abrt;
        I2C_nb_a_envoyer = 0;
        index_reception = 0;
        I2C_nb_a_recevoir = 0;
        statu_emission = TRANSACTION_TERMINEE;
        statu_reception = TRANSACTION_TERMINEE;
        i2c_statu_i2c0 = I2C_STATU_LIBRE;
        printf("Erreur acquitee\n");
    }

    while( (index_envoi < I2C_nb_a_envoyer) && (i2c_get_write_available(i2c)) ){
        bool restart = false;
        bool last = false;

        if (index_envoi == 0){
            // Début de l'envoi, assurons nous d'avoir la bonne adresse de l'esclave
            i2c->hw->enable = 0;
            i2c->hw->tar = adresse_7_bits;
            i2c->hw->enable = 1;
        }else{
            // Passage de l'écriture à la lecture, on envoie un bit de restart.
            if( !(I2C_tampon_envoi[index_envoi-1] & I2C_IC_DATA_CMD_CMD_BITS) && 
                    (I2C_tampon_envoi[index_envoi] & I2C_IC_DATA_CMD_CMD_BITS)){
                restart = true;
            }
        }

        if(index_envoi + 1 == I2C_nb_a_envoyer){
            // Fin de la trame, nous devons envoyer un bit de stop.
            last = true;
        }

        i2c->hw->data_cmd =
                I2C_tampon_envoi[index_envoi] |
                bool_to_bit(restart) << I2C_IC_DATA_CMD_RESTART_LSB |
                bool_to_bit(last) << I2C_IC_DATA_CMD_STOP_LSB;

        if(last){
            statu_emission = TRANSACTION_TERMINEE;
            index_envoi = 0;
            I2C_nb_a_envoyer = 0;
            //printf("I2C emission terminee\n");
        }else{
            index_envoi++;
        }
        
    }

    // Réception des données - Lecture des données présentes dans le tampon
    while( (index_reception < I2C_nb_a_recevoir) && (i2c_get_read_available(i2c)) ){
        I2C_tampon_reception[index_reception] = (uint8_t) i2c->hw->data_cmd;
        index_reception++;
    }
    if(index_reception == I2C_nb_a_recevoir && I2C_nb_a_recevoir > 0 ){
        statu_reception = TRANSACTION_TERMINEE;
        index_reception = 0;
        I2C_nb_a_recevoir = 0;
    }

    if(statu_reception == TRANSACTION_TERMINEE && statu_emission == TRANSACTION_TERMINEE){
        i2c_statu_i2c0 = I2C_STATU_LIBRE;
    }

}

/// @brief Charge le tampon d'émission pour pré-mâcher le travail à la fonction i2c_gestion
/// @param emission 
/// @param nb_envoi 
/// @param nb_reception 
void i2c_charger_tampon_envoi(uint8_t* emission, uint16_t nb_envoi, uint16_t nb_reception){
    // Données à envoyer
    for(unsigned int index=0; index<nb_envoi; index++){
        I2C_tampon_envoi[index] = (uint16_t) emission[index];
    }
    // Données à lire
    for(unsigned int index=0; index<nb_reception; index++){
        I2C_tampon_envoi[nb_envoi + index] = (uint16_t) 0x0100;
    }
}

/// @brief Stock l'adresse de l'esclave avec lequel communiquer
/// @param _adresse_7bits 
void i2d_set_adresse_esclave(uint8_t _adresse_7bits){
    adresse_7_bits =_adresse_7bits;
}

/// @brief Initialise la transmission I2, sur l'i2c0. Une transmission se compose de 2 trames I2C, une pour écrire (Adresse + données), une pour lire
/// Si nb_reception = 0, alors la trame pour lire ne sera pas envoyée.
/// @param emission : données à envoyer
/// @param nb_envoi : nombre de données à envoyer
/// @param nb_reception : nombre de données à recevoir
/// @return 1 en cas d'échec, 0 en cas de succès
enum i2c_resultat_t i2c_transmission(uint8_t _adresse_7bits, uint8_t* emission, uint16_t nb_envoi, uint16_t nb_reception){
    static enum m_statu_t{
        I2C_STATU_INIT,
        I2C_STATU_EN_COURS,
    }m_statu = I2C_STATU_INIT;

    switch(m_statu){
        case I2C_STATU_INIT:
            // I2C libre ?
            if(i2c_statu_i2c0 == I2C_STATU_OCCUPE){
                return I2C_EN_COURS;
            }
            // Alors il est à nous !
            i2c_statu_i2c0 = I2C_STATU_OCCUPE;
            statu_emission = TRANSACTION_EN_COURS;
            statu_reception = TRANSACTION_EN_COURS;
            i2c_error_code = 0;

            i2d_set_adresse_esclave(_adresse_7bits);

            i2c_charger_tampon_envoi(emission, nb_envoi, nb_reception);
            // Nous devons envoyer aussi une commande pour chaque octet à recevoir.
            I2C_nb_a_envoyer = nb_envoi + nb_reception;
            I2C_nb_a_recevoir = nb_reception;
            
            // On appelle le fonction gestion pour gagner du temps.
            i2c_gestion(i2c0);
            m_statu = I2C_STATU_EN_COURS;
            break;

        case I2C_STATU_EN_COURS:
            if(i2c_statu_i2c0 == I2C_STATU_LIBRE){
                m_statu = I2C_STATU_INIT;
                if(i2c_error_code){
                    return I2C_ECHEC;
                }else{
                    return I2C_SUCCES;
                }
                
            }
            break;
    }
    return I2C_EN_COURS;
}

/// @brief 
enum i2c_resultat_t i2c_lire_registre_nb(uint8_t adresse_7_bits, uint8_t registre, uint8_t * reception, uint8_t len){
    uint8_t emission[1];
    emission[0] = registre;
    enum i2c_resultat_t i2c_resultat;
    i2c_resultat = i2c_transmission(adresse_7_bits, emission, 1, len);
    if(i2c_resultat == I2C_SUCCES){
        for(uint32_t i = 0; i < len; i++){
            reception[i] = I2C_tampon_reception[i];
        }
        return I2C_SUCCES;
    }else if(i2c_resultat == I2C_ECHEC){
        return I2C_ECHEC;
    }
    return I2C_EN_COURS;
    
}



/// @brief Pour l'instant bloquant, mais devrait passer en non bloquant bientôt => Non, voir i2c_lire_registre_nb
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