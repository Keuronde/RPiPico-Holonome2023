#include "gyro_ADXRS453.h"
#include "spi_nb.h"
#include <stdio.h>


#ifdef GYRO_ADXRS453
#define NB_MAX_CHAR_GYRO 4

struct {
    unsigned short SQ:3;
    unsigned short ST:2;
    unsigned short P0:1;
    unsigned short P1:1;
    unsigned short PLL:1;
    unsigned short Q:1;
    unsigned short NVM:1;
    unsigned short POR:1;
    unsigned short PWR:1;
    unsigned short CST:1;
    unsigned short CHK:1;
    int16_t rateData;
} Gyro_SensorData;

void Gyro_traitementDonnees(unsigned char * tamponRecu);
unsigned char pariteOctet(unsigned char octet);

int gyro_spi_wr_32bits(uint16_t *transmit_buffer, uint8_t *recieve_buffer){
    int nb_recu;

    cs_select();
    if(spi_nb_write_data(spi0, (uint16_t*) transmit_buffer, 4) == SPI_ERR_TRANSMIT_FIFO_FULL){
        puts("gyro_spi_wr_32bits: SPI_ERR_TRANSMIT_FIFO_FULL");
    }else{
        while(spi_nb_busy(spi0));
        nb_recu = spi_nb_read_data_8bits(spi0, recieve_buffer);
    }
    if(nb_recu != 4){
        puts("gyro_spi_wr_32bits: nb_recu incohérent");
    }
    cs_deselect();
}

void affiche_tampon_32bits(uint8_t *tampon){
    uint32_t valeur;
    valeur = (tampon[0] << 24) + (tampon[1] << 16) + (tampon[2]<<8) + tampon[3];
    printf("Tampon: %#010x\n", valeur);

}


int gyro_get_sensor_data(uint16_t tampon_envoi[], uint8_t tampon_reception[]){
    tampon_envoi[0] = 0x30;
    tampon_envoi[1] = 0x00;
    tampon_envoi[2] = 0x00;
    tampon_envoi[3] = 0x01;
    gyro_spi_wr_32bits(tampon_envoi, tampon_reception);
    Gyro_traitementDonnees(tampon_reception);
    if(Gyro_SensorData.SQ != 0x4){
        printf("Gyro Failed - SQ bits (%#3x)!= 0x4\n", Gyro_SensorData.SQ);
        affiche_tampon_32bits(tampon_reception);
        return 1;
    }
    if(Gyro_SensorData.ST != 0x1){
        printf("Gyro Failed - Status (%#3x)!= 0x1\n", Gyro_SensorData.ST);
        affiche_tampon_32bits(tampon_reception);
        return 1;
    }
    return 0;
}

int gyro_init_check(){
    // Renvoi 0 si l'initialisation s'est bien passée
    // Renvoi 1 si le gyroscope n'a pas répondu
    uint16_t tampon_envoi[5]={0, 0, 0, 0, 0};
    uint8_t tampon_reception[5]="\0\0\0\0\0";

    // On suit les instructions de la page 20 de la fiche technique
    sleep_ms(100); // init du gyro - On ignore la réponse
    printf("T=100ms\n");
    tampon_envoi[0] = 0x30;
    tampon_envoi[1] = 0x00;
    tampon_envoi[2] = 0x00;
    tampon_envoi[3] = 0x02;
    
    gyro_spi_wr_32bits(tampon_envoi, tampon_reception);
    Gyro_traitementDonnees(tampon_reception);
    affiche_tampon_32bits(tampon_reception);

    
    sleep_ms(50); // t=150ms - On ignore, les données ne sont pas actualisées
    printf("T=150ms\n");
    tampon_envoi[0] = 0x30;
    tampon_envoi[1] = 0x00;
    tampon_envoi[2] = 0x00;
    tampon_envoi[3] = 0x01;
    
    gyro_spi_wr_32bits(tampon_envoi, tampon_reception);
    Gyro_traitementDonnees(tampon_reception);
    if(Gyro_SensorData.SQ != 0b100){
        printf("Gyro_Init - SQ bits (%#01x)!= 0x4", Gyro_SensorData.SQ);
        return 1;
    }
    affiche_tampon_32bits(tampon_reception);
   
    sleep_ms(50); // t=200ms - En cours d'autotest
    printf("T=200ms\n");
    tampon_envoi[0] = 0x30;
    tampon_envoi[1] = 0x00;
    tampon_envoi[2] = 0x00;
    tampon_envoi[3] = 0x01;
    
    gyro_spi_wr_32bits(tampon_envoi, tampon_reception);
    Gyro_traitementDonnees(tampon_reception);
    if(Gyro_SensorData.SQ != 0b100){
        printf("Gyro_Init - SQ bits (%#01x)!= 0x4", Gyro_SensorData.SQ);
        return 1;
    }
    affiche_tampon_32bits(tampon_reception);

    sleep_us(1); // t=200ms + TD - résultats de 200ms + TD, en cours d'autotest.
    printf("T=200ms+TD\n");
    tampon_envoi[0] = 0x30;
    tampon_envoi[1] = 0x00;
    tampon_envoi[2] = 0x00;
    tampon_envoi[3] = 0x01;
    
    gyro_spi_wr_32bits(tampon_envoi, tampon_reception);
    Gyro_traitementDonnees(tampon_reception);
    if(Gyro_SensorData.SQ != 0b100){
        printf("Gyro_Init - SQ bits (%#01x)!= 0x4", Gyro_SensorData.SQ);
        return 1;
    }
    affiche_tampon_32bits(tampon_reception);

    sleep_us(1); // t=200ms + 2TD - doit être nominal
    printf("T=200ms+2TD\n");
    tampon_envoi[0] = 0x00;
    tampon_envoi[1] = 0x00;
    tampon_envoi[2] = 0x00;
    tampon_envoi[3] = 0x00;
    if(gyro_get_sensor_data(tampon_envoi, tampon_reception)){
        return 1;
    }
    return 0;

}


int gyro_config(){
    return 0;
}



void gyro_get_vitesse_brute(struct t_angle_gyro* angle_gyro, struct t_angle_gyro* angle_gyro_moy){
    uint16_t tampon_envoi[5]={0, 0, 0, 0, 0};
    uint8_t tampon_reception[5]="\0\0\0\0\0";
    int16_t rot_z;
    
    sleep_us(1); // A supprimer plus tard
    if(gyro_get_sensor_data(tampon_envoi, tampon_reception)){
        return;
    }
    
    rot_z = Gyro_SensorData.rateData;

    if(angle_gyro_moy == NULL){
        angle_gyro->rot_x = 0;
        angle_gyro->rot_y = 0;
        angle_gyro->rot_z = rot_z * 32;
    }else{
        angle_gyro->rot_x = 0;
        angle_gyro->rot_y = 0;
        angle_gyro->rot_z = (int32_t) rot_z * 32 - angle_gyro_moy->rot_z;
    }
}

void gyro_get_vitesse_normalisee(struct t_angle_gyro* _vitesse_angulaire,
        struct t_angle_gyro_double * _vitesse_gyro){
    _vitesse_gyro->rot_x = (double)_vitesse_angulaire->rot_x * 0.0125 / 32.0;
    _vitesse_gyro->rot_y = (double)_vitesse_angulaire->rot_y * 0.0125 / 32.0;
    _vitesse_gyro->rot_z = (double)_vitesse_angulaire->rot_z * 0.0125 / 32.0;
}




inline unsigned char Gyro_commande_SensorData(unsigned char autotest){
    // On met SQ2 à 1 afin de différencier facilement une erreur et des données
    uint8_t tamponGyroscopeEnvoi[4];
    tamponGyroscopeEnvoi[0] = 0x30;
    tamponGyroscopeEnvoi[1] = 0x00;
    tamponGyroscopeEnvoi[2] = 0x00;
    if (autotest){
        tamponGyroscopeEnvoi[3] = 0x03;
    }else{
        tamponGyroscopeEnvoi[3] = 0x01;
    }
    // La parité, dans ce cas est triviale, autant prévoir tous les cas
    //Gyro_commande_PariteData(tamponGyroscopeEnvoi);
    //return SPI_envData(tamponGyroscopeEnvoi);
}

void Gyro_commande_PariteData(unsigned char* tampon){
    unsigned char parite=0,i;
    // Obtention de la parité actuelle
    for(i=0 ; i< NB_MAX_CHAR_GYRO ; i++){
        parite ^= pariteOctet(tampon[i]);
    }
    // On veut une parité impaire
    parite ^= 0x01;

    // On insere ce bit dans le message, au bon endroit
    tampon[NB_MAX_CHAR_GYRO-1] = tampon[NB_MAX_CHAR_GYRO-1] | parite;
}

unsigned char pariteOctet(unsigned char octet){
    unsigned char parite=0,i;
    for (i=0 ; i<8 ; i++){
        parite ^= octet & 0x01;
        octet = octet >> 1;
    }
    return parite;
}

void Gyro_traitementDonnees(uint8_t * tamponRecu){
    Gyro_SensorData.SQ = (tamponRecu[0]>>5) & 0x07;
    Gyro_SensorData.P0 = (tamponRecu[0]>>4) & 0x01;
    Gyro_SensorData.ST = (tamponRecu[0]>>2) & 0x03;
    Gyro_SensorData.rateData = (int) 
            ( (0xC000 &((unsigned int) (tamponRecu[0] & 0x03)<<14)) |
            ( 0x3FC0 & ((unsigned int) tamponRecu[1] << 6)) |
           ( 0x003F & (unsigned int) ( tamponRecu[2] >> 2)));
    Gyro_SensorData.PLL = (tamponRecu[3] & 0x80) >> 7;
    Gyro_SensorData.Q   = (tamponRecu[3] & 0x40) >> 6;
    Gyro_SensorData.NVM = (tamponRecu[3] & 0x20) >> 5;
    Gyro_SensorData.POR = (tamponRecu[3] & 0x10) >> 4;
    Gyro_SensorData.PWR = (tamponRecu[3] & 0x08) >> 3;
    Gyro_SensorData.CST = (tamponRecu[3] & 0x04) >> 2;
    Gyro_SensorData.CHK = (tamponRecu[3] & 0x02) >> 1;
    Gyro_SensorData.P1  = (tamponRecu[3] & 0x01);

}
/*
unsigned char Gyro_gestion(void){
    Gyro_commande_SensorData(0);
    while(!SPI_recData(GyroscopeReception));
    
    Gyro_traitementDonnees(GyroscopeReception);
    if (Gyro_SensorData.SQ & 0x04){
        Gyro_Angle +=(long) (Gyro_SensorData.rateData - angle0);
        //Gyro_Angle = angle0;
    }else{
        Gyro_Angle = (long)0x3333;
    }
    return 0;
}
inline unsigned char Gyro_gestion_nb(){
    if(SPI_recData(GyroscopeReception)){
        Gyro_traitementDonnees(GyroscopeReception);
        if (Gyro_SensorData.SQ & 0x04)
            // calcul du nouvel angle
            // TODO : Améliorer la stabilitée en augmentant la précision
            Gyro_Angle +=(long) ((long)Gyro_SensorData.rateData - (long)angle0);
        return 0;
    }
    return 1;

}

int Gyro_getAngle(void){
    // 80° par secondes
    // 5 kHz => 200 µs
    // Gyro_Angle en 1,6 e-2 degré
    return (int)(-Gyro_Angle / 5000 / 80);
}
long Gyro_getRawAngle(void){
    // 80° par secondes
    // 5 kHz => 200 µs
    // Gyro_Angle en 1,6 e-2 degré
    return Gyro_Angle ;
}
long double Gyro_getAngleRadian(void){
    // 80° par secondes
    // 5 kHz => 200 µs
    // Gyro_Angle en 1,6 e-2 degré
    return -Gyro_Angle * GYRO_COEF_RADIAN_5kHz;
}
unsigned char * Gyro_getRawData(){
    return GyroscopeReception;
}

int Gyro_init(){
    
    long long calcul_angle0;
    
    int i, erreur_gyro;
    
    Gyro_Timer_ms=100;
    while(Gyro_Timer_ms);
    // Envoie message auto-test des test
    while(!Gyro_commande_SensorData(1));
    while(!SPI_recData(GyroscopeReception));
    
    // Attente 50 ms - les tests doivent indiquer des erreur
    Gyro_Timer_ms=50;
    while(Gyro_Timer_ms);
    while(!Gyro_commande_SensorData(0));
    while(!SPI_recData(GyroscopeReception));

    // Attente 50 ms - les erreurs doivent s'être effacées
    Gyro_Timer_ms=50;
    while(Gyro_Timer_ms);
    while(!Gyro_commande_SensorData(0));
    while(!SPI_recData(GyroscopeReception));

    // Calibration du gyroscope
    calcul_angle0 = 0;
    i=0;
    erreur_gyro = 0;
    while((i<NB_ACQ_CALIBRATION) && (erreur_gyro < NB_MAX_ERREUR_GYRO)){
        while(!Gyro_commande_SensorData(0));
        while(!SPI_recData(GyroscopeReception));
        Gyro_traitementDonnees(GyroscopeReception);
        if (Gyro_SensorData.SQ & 0x04){
            calcul_angle0 += Gyro_SensorData.rateData;
            erreur_gyro = 0;
            i++;
        }else{
            erreur_gyro++;
        }
        __delay32(2000); // 50 µs
    }
    if (erreur_gyro < NB_MAX_ERREUR_GYRO){
        erreur_gyro = 0;
        // TODO : Améliorer la stabilitée en augmentant la précision
        angle0 = (long)(calcul_angle0 / NB_ACQ_CALIBRATION);
        Gyro_Pret=1;
    }else{
        erreur_gyro = 1;
    }
    

    return erreur_gyro;
}
*/
#endif