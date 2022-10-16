#include "gyro_ADXRS473.h"
#include "spi_nb.h"
#include <stdio.h>

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
    signed int rateData;
} Gyro_SensorData;

void Gyro_traitementDonnees(unsigned char * tamponRecu);
unsigned char pariteOctet(unsigned char octet);

int gyro_read_register_blocking(uint8_t registrer, uint8_t *tampon, uint8_t nb_a_lire){
    uint8_t tampon_envoi[4]="\0\0\0\0";
    int nb_recu;
    tampon_envoi[0] = registrer;

    // Envoie commande N
    cs_select();
    spi_write_blocking(spi0, tampon_envoi, 4);
    nb_recu = spi_read_blocking(spi0, 0, tampon, nb_a_lire);
    cs_deselect();

    // lire reponse N
    cs_select();
    spi_write_blocking(spi0, tampon_envoi, 4);
    nb_recu = spi_read_blocking(spi0, 0, tampon, nb_a_lire);
    cs_deselect();
    
}

int gyro_init_check(){
    // Renvoi 0 si l'initialisation s'est bien passée
    // Renvoi 1 si le gyroscope n'a pas répondu
    uint8_t tampon[5]="\0\0\0\0\0";
    gyro_read_register_blocking(0x0C, tampon, 1);
    Gyro_traitementDonnees(tampon);

    printf("Init check : %#06x\n", Gyro_SensorData.rateData);
    
/*    if(tampon[0] == 0xd7){
        return 0;
    }*/
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

void Gyro_traitementDonnees(unsigned char * tamponRecu){
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