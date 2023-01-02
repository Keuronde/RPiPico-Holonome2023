#include <stdio.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "math.h"
#include "Test.h"

#include "APDS_9960.h"
#include "gyro.h"
#include "Asser_Moteurs.h"
#include "Asser_Position.h"
#include "Commande_vitesse.h"
#include "i2c_maitre.h"
#include "Localisation.h"
#include "Moteurs.h"
#include "QEI.h"
#include "Robot_config.h"
#include "Servomoteur.h"
#include "spi_nb.h"
#include "Temps.h"
#include "Trajectoire.h"
#include "Trajet.h"

#define V_INIT -999.0
#define TEST_TIMEOUT_US 10000000

int test_APDS9960(void);
int test_moteurs(void);
int test_QIE(void);
int test_QIE_mm(void);
int test_vitesse_moteur(enum t_moteur moteur);
int test_asser_moteur(void);
int test_localisation(void);
int test_avance(void);
int test_cde_vitesse_rotation(void);
int test_cde_vitesse_rectangle(void);
int test_cde_vitesse_cercle(void);
int test_asser_position_avance(void);
int test_asser_position_avance_et_tourne(int);
int test_trajectoire(void);
int test_i2c_bus(void);
void affiche_localisation(void);
int test_i2c_lecture_pico_annex();
int test_i2c_lecture_pico_annex_nb();


// Mode test : renvoie 0 pour quitter le mode test
int mode_test(){
    static int iteration = 2;
    printf("Appuyez sur une touche pour entrer en mode test :\n");
    printf("A - pour asser_moteurs (rotation)\n");
    printf("B - pour avance (asser_moteur)\n");
    printf("C - pour les codeurs\n");
    printf("D - pour les codeurs (somme en mm)\n");
    printf("E - Commande en vitesse - rotation pure\n");
    printf("F - Commande en vitesse - carré\n");
    printf("G - Commande en vitesse - cercle\n");
    printf("H - Asser Position - avance\n");
    printf("I - Asser Position - avance et tourne (gyro)\n");
    printf("J - Asser Position - avance et tourne (sans gyro)\n");
    printf("M - pour les moteurs\n");
    printf("L - pour la localisation\n");
    printf("T - Trajectoire\n");
    printf("U - Scan du bus i2c\n");
    printf("V - APDS_9960\n");
    printf("W - Com i2c Pico Annexe\n");
    printf("X - Com i2c Pico Annexe - non bloquant\n");
    stdio_flush();
    int rep = getchar_timeout_us(TEST_TIMEOUT_US);
    stdio_flush();
    switch (rep)
    {
    case 'a':
    case 'A':
        while(test_asser_moteur());
        break;
    case 'b':
    case 'B':
        while(test_avance());
        break;
        
    case 'C':
    case 'c':
        while(test_QIE());
        break;

    case 'D':
    case 'd':
        while(test_QIE_mm());
        break;

    case 'E':
    case 'e':
        while(test_cde_vitesse_rotation());
        break;

    case 'F':
    case 'f':
        while(test_cde_vitesse_rectangle());
        break;

    case 'G':
    case 'g':
        while(test_cde_vitesse_cercle());
        break;

    case 'H':
    case 'h':
        while(test_asser_position_avance());
        break;

    case 'I':
    case 'i':
        while(test_asser_position_avance_et_tourne(1));
        break;
    
    case 'J':
    case 'j':
        while(test_asser_position_avance_et_tourne(0));
        break;
        
    case 'M':
    case 'm':
        while(test_moteurs());
        break;
    case 'L':
    case 'l':
        while(test_localisation());
        break;

    case 'T':
    case 't':
        while(test_trajectoire());
        break;

    case 'U':
    case 'u':
        while(test_i2c_bus());
        break;

    case 'V':
    case 'v':
        while(test_APDS9960());
        break;

    case 'W':
    case 'w':
        while(test_i2c_lecture_pico_annex());
        break;

    case 'X':
    case 'x':
        while(test_i2c_lecture_pico_annex_nb());
        break;

    case PICO_ERROR_TIMEOUT:
        iteration--;        
        if(iteration == 0){
            //printf("Sortie du mode test\n");
            //return 0;
        }

    default:
        printf("Commande inconnue\n");
        break;
    }
    return 1;
    
}

int test_continue_test(){
    int lettre;
    printf("q pour quitter, une autre touche pour un nouveau test.\n");
    do{
        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT || lettre == 0);
    switch(lettre){
        case 'q':
        case 'Q':
            return 0;
        default:
            return 1;
    }

}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int test_APDS9960(){
    int lettre;

    printf("Initialisation\n");
    APDS9960_Init();
    printf("Lecture...\n");
    /*
    do{
        APDS9960_Lire();
        lettre = getchar_timeout_us(0);
        stdio_flush();
    }while(lettre == PICO_ERROR_TIMEOUT);*/
    while(1){
        APDS9960_Lire();
        sleep_ms(100);
    }
    return 1;
}

int test_i2c_lecture_pico_annex(){
    i2c_maitre_init();
    uint8_t tampon[10];
    uint8_t registre=0;
    uint8_t adresse = 0x17;
    int ret; 
    
    ret = i2c_write_blocking(i2c0, adresse,&registre, 1, false);
    if(ret < 0){
        printf("Erreur I2C : %d", ret);
        return 0;
    }

    ret = i2c_read_blocking(i2c_default, adresse, tampon, 10, false);
    if(ret < 0){
        printf("Erreur I2C : %d", ret);
    }else{
        for(int i=0; i<10; i++){
            printf("%c", tampon[i]);
        }
        printf("\n");

        for(int i=0; i<10; i++){
            printf("%2x ", tampon[i]);
        }
        printf("\n");
    }
    return test_continue_test();
}

int test_i2c_lecture_pico_annex_nb(){
    i2c_maitre_init();
    uint8_t tampon[10];
    uint8_t registre=0;
    uint8_t adresse = 0x17;
    uint32_t time_i2c[5];
    const uint8_t T_MAX_I2C = 10;
    int ret; 

    time_i2c[0] = time_us_32();

    // On charge l'adresse de l'escalve
    i2c0->hw->enable = 0;
    i2c0->hw->tar = adresse;
    i2c0->hw->enable = 1;

    // On envoie l'adresse du registre à lire
    // Pas de stop, pas de restart, écriture : 0, 

    i2c0->hw->data_cmd = registre;

    uint8_t first = false;
    uint8_t last = false;

    for(int i=0; i<T_MAX_I2C; i++){
        first = false;
        last = false;
        if (i == 0){
            first = true;
        }
        if(i == T_MAX_I2C -1){
            last = true;
        }

        i2c0->hw->data_cmd =
                bool_to_bit(first) << I2C_IC_DATA_CMD_RESTART_LSB |
                bool_to_bit(last) << I2C_IC_DATA_CMD_STOP_LSB |
                I2C_IC_DATA_CMD_CMD_BITS; // -> 1 for read
    }

    time_i2c[1] = time_us_32() - time_i2c[0] ;

    // On attend la fin de la transaction i2c
    while(i2c0->hw->status & I2C_IC_STATUS_MST_ACTIVITY_BITS);

    time_i2c[2] = time_us_32() - time_i2c[0] ;

    // On lit le tampon I2C
    // uint8_t * dst;
    // dst = tampon;
    
    for(int i=0; i<T_MAX_I2C; i++){
        // On attend une donnée
        while(!i2c_get_read_available(i2c0));

        // Code erreur
        if(i2c0->hw->tx_abrt_source){
            printf("Erreur I2C: Abort : %4x\n", i2c0->hw->tx_abrt_source);
        }

        //On lit la donnée
        tampon[i] = (uint8_t) i2c0->hw->data_cmd;
    }

    time_i2c[3] = time_us_32() - time_i2c[0] ;

    // Affichage
    for(int i=0; i<T_MAX_I2C; i++){
        printf("%c", tampon[i]);
    }
    printf("\n");

    for(int i=0; i<T_MAX_I2C; i++){
        printf("%2x ", tampon[i]);
    }
    printf("\n");

    printf("T_init: %u, T_attente: %u, T_lecture: %u\n", time_i2c[1], time_i2c[2], time_i2c[3]);

    return test_continue_test();
}

int test_i2c_bus(){
    // Adresse I2C : 0b0100 000 R/W
    // Lecture des broches sur les registres 0 et 1
    // Registre 2 et 3 : valeur des broches en sorties
    // Registre 4 et 5 : INversion de polarité
    // Registre 6 et 7 : Configuration entrée (1) ou sortie (0)

    uint8_t reception[8];
    uint8_t emission[8];
    //uint8_t adresse = 0b0100000;
    uint8_t adresse = 0x20;
    int statu;
    int lettre;

    emission[0]=6; // Registre à lire

    i2c_maitre_init();
    // Scan bus I2C - cf SDK
    printf("\nI2C Bus Scan\n");
    printf("   0 1 2 3 4 5 6 7 8 9 A B C D E F\n");
    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }
        int ret;
        uint8_t rxdata=0x55;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
            //ret = i2c_write_blocking(i2c_default, addr, &rxdata, 1, false);
        
        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : " ");
    }
    printf("Done.\n");
    return 0;

    do{    
        statu = i2c_write_blocking (i2c0, adresse, emission, 1, 0);
        if(statu == PICO_ERROR_GENERIC){
            printf("Emission : Address not acknowledged, no device present.\n");
            return 0;
        }else{
            printf("Emission : Ok\n");
        }

        statu = i2c_read_blocking(i2c0, adresse, reception, 2, 0);
        if(statu == PICO_ERROR_GENERIC){
            printf("Reception : Address not acknowledged, no device present.\n");
            return 0;
        }else{
            printf("Recetion : Ok\n");
        }
        printf("%2.x%2.x\n",reception[0], reception[1]);
        
        lettre = getchar_timeout_us(0);
        stdio_flush();
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;

}

void test_trajectoire_printf(){
    struct position_t _position;
    while(1){
        _position  = Trajet_get_consigne();
        printf("T: %ld, X: %f, Y: %f, orientation: %2.1f\n", time_us_32()/1000, _position.x_mm, _position.y_mm, _position.angle_radian/M_PI*180);
    }

}

int test_trajectoire(){
    int lettre, _step_ms = 1, temps_ms=0;
    Trajet_init();
    struct trajectoire_t trajectoire;
    printf("Choix trajectoire :\n");
    printf("B - Bezier\n");
    printf("C - Circulaire\n");
    printf("D - Droite\n");
    do{
         lettre = getchar_timeout_us(TEST_TIMEOUT_US);
         stdio_flush();
    }while(lettre == PICO_ERROR_TIMEOUT);
    switch(lettre){
        case 'b':
        case 'B':
            Trajectoire_bezier(&trajectoire, 0, 0, -200., 450, 250, 450, 0, 0);
            break;

        case 'c':
        case 'C':
            Trajectoire_circulaire(&trajectoire, 0, 250, -90, 90, 250);
            break;

        case 'd':
        case 'D':
            Trajectoire_droite(&trajectoire, 0, 0, 0, 700);
            break;
        
        default: return 0;
    }

    Trajet_debut_trajectoire(trajectoire);
    multicore_launch_core1(test_trajectoire_printf);
    do{
        // Routines à 1 ms
        QEI_update();
        Localisation_gestion();
        
        if(Trajet_avance(_step_ms/1000.) == TRAJET_TERMINE){
            Moteur_SetVitesse(MOTEUR_A, 0);
            Moteur_SetVitesse(MOTEUR_B, 0);
            Moteur_SetVitesse(MOTEUR_C, 0);
        }else{
            AsserMoteur_Gestion(_step_ms);
        }
        sleep_ms(_step_ms);
        temps_ms += _step_ms;
        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;
    
}

/// @brief Avance droit 100 mm/s en tournant sur lui-même (1rad/s)
/// @param m_gyro : 1 pour utiliser le gyroscope, 0 sans
/// @return 
int test_asser_position_avance_et_tourne(int m_gyro){
    int lettre, _step_ms = 1, _step_ms_gyro = 2, step_gyro=2;
    uint32_t temps_ms = 0, temps_ms_init = 0, temps_ms_old;
    struct position_t position_consigne;

    position_consigne.angle_radian = 0;
    position_consigne.x_mm = 0;
    position_consigne.y_mm = 0;
    
    printf("Le robot avance à 100 mm/s\n");
    printf("Init gyroscope\n");
    Gyro_Init();
    printf("C'est parti !\n");
    stdio_flush();

    set_position_avec_gyroscope(m_gyro);
    temps_ms = Temps_get_temps_ms();
    temps_ms_old = temps_ms;
    temps_ms_init = temps_ms;

    multicore_launch_core1(affiche_localisation);
    do{
        while(temps_ms == Temps_get_temps_ms());
        temps_ms = Temps_get_temps_ms();
        temps_ms_old = temps_ms;

        QEI_update();
        if(temps_ms % _step_ms_gyro == 0){
            Gyro_Read(_step_ms_gyro);
        }
        Localisation_gestion();
        AsserMoteur_Gestion(_step_ms);
        
        position_consigne.angle_radian = (double) (temps_ms - temps_ms_init) /1000.;
        position_consigne.y_mm = (double) (temps_ms - temps_ms_init) * 100. / 1000.;

        Asser_Position(position_consigne);

        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT || lettre == 0);

    printf("lettre : %c, %d\n", lettre, lettre);

    return 0;
}

int test_asser_position_avance(){
    int lettre, _step_ms = 1, temps_ms=0;
    struct position_t position;

    position.angle_radian = 0;
    position.x_mm = 0;
    position.y_mm = 0;
    
    printf("Le robot avance à 100 mm/s\n");
    do{
        QEI_update();
        Localisation_gestion();
        AsserMoteur_Gestion(_step_ms);
        
        if(temps_ms < 5000){
            position.y_mm = (double) temps_ms * 100. / 1000.;
        }else if(temps_ms < 10000){ 
            position.y_mm = 1000 - (double) temps_ms * 100. / 1000.;
        }else{
            temps_ms = 0;
        }

        Asser_Position(position);
        temps_ms += _step_ms;
        sleep_ms(_step_ms);

        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;
}

int test_cde_vitesse_rotation(){
    int lettre, _step_ms = 1;
    double vitesse =90.0/2 * 3.14159 /180.0;
    printf("Rotation du robot sur lui-même en 8 secondes\nVitesse : %f rad/s\n", vitesse);

    commande_vitesse(0, 0, vitesse);
    do{
        QEI_update();
        AsserMoteur_Gestion(_step_ms);
        sleep_ms(_step_ms);
        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;
}

int test_cde_vitesse_rectangle(){
    int lettre, _step_ms = 1, temps_ms=0;

    printf("déplacement en rectangle du robot : 500x200 mm, 100 mm/s\n");
    do{
        QEI_update();
        AsserMoteur_Gestion(_step_ms);

        if(temps_ms < 5000){
            commande_vitesse(0, 100, 0);
        }else if(temps_ms < 7000){
            commande_vitesse(-100, 0, 0);
        }else if(temps_ms < 12000){
            commande_vitesse(0, -100, 0);
        }else if(temps_ms < 14000){
            commande_vitesse(100, 0, 0);
        }else{
            temps_ms = 0;
        }
        
        sleep_ms(_step_ms);
        temps_ms += _step_ms;
        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;
}

int test_cde_vitesse_cercle(){
    int lettre, _step_ms = 1, temps_ms=0;

    printf("déplacement en cercle du robot : 100 mm/s\n");
    do{
        QEI_update();
        AsserMoteur_Gestion(_step_ms);
        commande_vitesse(cos((double)temps_ms / 1000.) * 200.0, sin((double)temps_ms /1000.) * 200.0, 0);
        temps_ms += _step_ms;
        sleep_ms(_step_ms);
        

        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;
}

int test_avance(void){
    int lettre;
    int _step_ms = 1;
    AsserMoteur_setConsigne_mm_s(MOTEUR_A, -100);
    AsserMoteur_setConsigne_mm_s(MOTEUR_B, 100);
    AsserMoteur_setConsigne_mm_s(MOTEUR_C, 0);
    
    do{
        QEI_update();
        AsserMoteur_Gestion(_step_ms);
        sleep_ms(_step_ms);
        lettre = getchar_timeout_us(0);

    }while(lettre == PICO_ERROR_TIMEOUT);
    Moteur_SetVitesse(MOTEUR_A, 0);
    Moteur_SetVitesse(MOTEUR_B, 0);
    Moteur_SetVitesse(MOTEUR_C, 0);
    return 0;
}

void affiche_localisation(){
    struct position_t position;
    while(1){
        position = Localisation_get();
        printf("X: %f, Y: %f, angle: %f\n", position.x_mm, position.y_mm, position.angle_radian *180. / 3.141592654);

    }
}

void test_asser_moteur_printf(){
    int _step_ms = 1;
    while(1){
        printf("Vitesse A : %.0f, vitesse B : %.0f, vitesse C : %.0f\n", AsserMoteur_getVitesse_mm_s(MOTEUR_A, _step_ms),
            AsserMoteur_getVitesse_mm_s(MOTEUR_B, _step_ms), AsserMoteur_getVitesse_mm_s(MOTEUR_C, _step_ms));
        //sleep_ms(5);
    }
}

int test_asser_moteur(){
    int lettre;
    int _step_ms = 1;
    printf("Asservissement des moteurs :\nAppuyez sur une touche pour quitter\n");
    AsserMoteur_setConsigne_mm_s(MOTEUR_A, 100);
    AsserMoteur_setConsigne_mm_s(MOTEUR_B, 100);
    AsserMoteur_setConsigne_mm_s(MOTEUR_C, 100);
    multicore_launch_core1(test_asser_moteur_printf);
    do{
        QEI_update();
        AsserMoteur_Gestion(_step_ms);
        sleep_ms(_step_ms);
        //printf("Vitesse A : %d, codeur B : %d, codeur C : %d\n", QEI_get(QEI_A_NAME), QEI_get(QEI_B_NAME), QEI_get(QEI_C_NAME));
        //printf("Vitesse A : %.0f, vitesse B : %.0f, vitesse C : %.0f\n", AsserMoteur_getVitesse_mm_s(MOTEUR_A, _step_ms),
        //    AsserMoteur_getVitesse_mm_s(MOTEUR_B, _step_ms), AsserMoteur_getVitesse_mm_s(MOTEUR_C, _step_ms));
        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);
    Moteur_SetVitesse(MOTEUR_A, 0);
    Moteur_SetVitesse(MOTEUR_B, 0);
    Moteur_SetVitesse(MOTEUR_C, 0);
    multicore_reset_core1();
    return 0;
}

int test_QIE(){
    int lettre;
    printf("Affichage des QEI :\nAppuyez sur une touche pour quitter\n");
    do{
        QEI_update();
        printf("Codeur A : %d (%3.2f mm), codeur B : %d (%3.2f mm), codeur C : %d (%3.2f mm)\n", 
            QEI_get(QEI_A_NAME), QEI_get_mm(QEI_A_NAME),
            QEI_get(QEI_B_NAME), QEI_get_mm(QEI_B_NAME),
            QEI_get(QEI_C_NAME), QEI_get_mm(QEI_C_NAME));
        sleep_ms(100);

        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);
    return 0;

}

int test_QIE_mm(){
    int lettre;
    printf("Affichage des QEI :\nAppuyez sur une touche pour quitter\n");
    double a_mm=0, b_mm=0, c_mm=0;
    do{
        QEI_update();
        a_mm += QEI_get_mm(QEI_A_NAME);
        b_mm += QEI_get_mm(QEI_B_NAME);
        c_mm += QEI_get_mm(QEI_C_NAME);
        printf("Codeur A : %3.2f mm, codeur B : %3.2f mm, codeur C : %3.2f mm\n", a_mm, b_mm, c_mm);
        sleep_ms(100);

        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);
    return 0;

}

int test_localisation(){
    int lettre;
    struct position_t position;
    
    printf("Affichage de la position du robot.\nAppuyez sur une touche pour quitter\n");
    do{
        QEI_update();
        Localisation_gestion();
        position = Localisation_get();
        printf("X: %f, Y: %f, angle: %f\n", position.x_mm, position.y_mm, position.angle_radian *180. / 3.141592654);
        sleep_ms(100);

        lettre = getchar_timeout_us(0);
    }while(lettre == PICO_ERROR_TIMEOUT);

    return 0;
    
}

int test_moteurs(){
    int lettre_moteur;

    printf("Indiquez le moteurs à tester (A, B ou C):\n");
    do{
         lettre_moteur = getchar_timeout_us(TEST_TIMEOUT_US);
         stdio_flush();
    }while(lettre_moteur == PICO_ERROR_TIMEOUT);
    printf("Moteur choisi : %c %d %x\n", lettre_moteur, lettre_moteur, lettre_moteur);

    switch (lettre_moteur)
    {
    case 'A':
    case 'a':
        while(test_vitesse_moteur(MOTEUR_A));
        break;

    case 'B':
    case 'b':
        while(test_vitesse_moteur(MOTEUR_B));
        break;

    case 'C':
    case 'c':
        while(test_vitesse_moteur(MOTEUR_C));
        break;
    
    case 'Q':
    case 'q':
        return 0;
        break;
    
    default:
        break;
    }
    
    return 1;
}

int test_vitesse_moteur(enum t_moteur moteur){
    printf("Vitesse souhaitée :\n0 - 0%%\n1 - 10%%\n2 - 20%%\n...\n9 - 90%%\nA - 100%%\n");

    int vitesse_moteur;
    do{ 
        vitesse_moteur = getchar_timeout_us(TEST_TIMEOUT_US);
        stdio_flush();
    }while(vitesse_moteur == PICO_ERROR_TIMEOUT);
    
    switch (vitesse_moteur)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        printf("Vitesse choisie : %c0%%\n", vitesse_moteur);
        Moteur_SetVitesse(moteur, (vitesse_moteur - '0') * 32767.0 / 10.);
        break;

    case 'A':
    case 'a':
        printf("Vitesse choisie : 100%%\n");
        Moteur_SetVitesse(moteur, (int16_t) 32766.0);
        break;

    case 'b':
    case 'B':
        printf("Vitesse choisie : -50%%\n");
        Moteur_SetVitesse(moteur, (int16_t) -32766.0/2);
        break;

    case 'q':
    case 'Q':
        return 0;
        break;

    default:
        break;
    }
    return 1;
}
