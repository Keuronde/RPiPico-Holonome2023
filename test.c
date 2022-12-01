#include <stdio.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "gyro.h"
#include "Temps.h"
#include "spi_nb.h"
#include "Servomoteur.h"
#include "Moteurs.h"
#include "QEI.h"
#include "Asser_Moteurs.h"
#include "Localisation.h"

const uint LED_PIN = 25;
const uint LED_PIN_ROUGE = 28;
const uint LED_PIN_NE_PAS_UTILISER = 22;


#define V_INIT -999.0
#define TEST_TIMEOUT_US 10000000

int mode_test();
int test_moteurs();
int test_QIE();
int test_QIE_mm();
int test_vitesse_moteur(enum t_moteur moteur);
int test_asser_moteur(void);
int test_localisation(void);
int test_avance(void);

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    double vitesse_filtre_x=V_INIT, vitesse_filtre_y=V_INIT, vitesse_filtre_z=V_INIT;
    struct t_angle_gyro_double angle_gyro;

    uint32_t temps_ms = 0, temps_ms_old;

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    gpio_init(LED_PIN_ROUGE);
    gpio_set_dir(LED_PIN_ROUGE, GPIO_OUT);
    gpio_put(LED_PIN_ROUGE, 1);

    // Il fuat neutraliser cettte broche qui pourrait interférer avec 
    // la lecture des codeurs. (problème sur la carte électrique)...
    gpio_init(LED_PIN_NE_PAS_UTILISER);
    gpio_set_dir(LED_PIN_NE_PAS_UTILISER, GPIO_IN);

    sleep_ms(3000);
    Servomoteur_Init();
    //puts("Debut");
    //spi_test();

    //while(1);
    Temps_init();
    Moteur_Init();
    QEI_init();
    AsserMoteur_Init();
    Localisation_init();

    while(mode_test());

    Gyro_Init();

    

    temps_ms = Temps_get_temps_ms();
    temps_ms_old = temps_ms;
    while (1) { 
        u_int16_t step_ms = 2;
        float coef_filtre = 1-0.8;
        
        while(temps_ms == Temps_get_temps_ms());
        temps_ms = Temps_get_temps_ms();
        temps_ms_old = temps_ms;
        
        // Tous les pas de step_ms
        if(!(temps_ms % step_ms)){
            Gyro_Read(step_ms);
            
            //gyro_affiche(gyro_get_vitesse(), "Angle :");
            // Filtre 
            angle_gyro = gyro_get_vitesse();
            if(vitesse_filtre_x == V_INIT){
                vitesse_filtre_x = angle_gyro.rot_x;
                vitesse_filtre_y = angle_gyro.rot_y;
                vitesse_filtre_z = angle_gyro.rot_z;
            }else{
                vitesse_filtre_x = vitesse_filtre_x * (1-coef_filtre) + angle_gyro.rot_x * coef_filtre;
                vitesse_filtre_y = vitesse_filtre_y * (1-coef_filtre) + angle_gyro.rot_y * coef_filtre;
                vitesse_filtre_z = vitesse_filtre_z * (1-coef_filtre) + angle_gyro.rot_z * coef_filtre;
            }

            //printf("%#x, %#x\n", (double)temps_ms_old / 1000,  vitesse_filtre_z);

            //printf("%d, %d\n", temps_ms, (int32_t) (vitesse_filtre_z * 1000));
            //gyro_affiche(angle_gyro, "Vitesse (°/s),");
            
        }

        // Toutes les 50 ms
        if((Temps_get_temps_ms() % 50) == 0){
            struct t_angle_gyro_double m_gyro;
            m_gyro = gyro_get_angle();
            printf("%f, %f\n", (double)temps_ms / 1000,  m_gyro.rot_z);
        }

        // Toutes les 500 ms
        if((Temps_get_temps_ms() % 500) == 0){
            //gyro_affiche(gyro_get_angle(), "Angle :");
        }
        // Toutes les secondes
        if((Temps_get_temps_ms() % 500) == 0){
            //gyro_get_temp();
        }
    }
}

// Mode test : renvoie 0 pour quitter le mode test
int mode_test(){
    static int iteration = 3;
    printf("Appuyez sur une touche pour entrer en mode test :\n");
    printf("A - pour asser_moteurs (rotation)\n");
    printf("B - pour avance (asser_moteur)\n");
    printf("C - pour les codeurs\n");
    printf("D - pour les codeurs (somme en mm)\n");
    printf("M - pour les moteurs\n");
    printf("L - pour la localisation\n");
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
        
    case 'M':
    case 'm':
        /* code */
        while(test_moteurs());
        break;
    case 'L':
    case 'l':
        /* code */
        while(test_localisation());
        break;
    case PICO_ERROR_TIMEOUT:
        iteration--;        
        if(iteration == 0){
            printf("Sortie du mode test\n");
            return 0;
        }

    default:
        printf("Commande inconnue\n");
        break;
    }
    return 1;
    
}


int test_avance(void){
    int lettre;
    int _step_ms = 1;
    AsserMoteur_setConsigne_mm_s(MOTEUR_A, 500);
    AsserMoteur_setConsigne_mm_s(MOTEUR_B, -500);
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
    AsserMoteur_setConsigne_mm_s(MOTEUR_A, 500);
    AsserMoteur_setConsigne_mm_s(MOTEUR_B, 500);
    AsserMoteur_setConsigne_mm_s(MOTEUR_C, 500);
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
