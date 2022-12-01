#include "QEI.h"
#include "Moteurs.h"
#include "Asser_Moteurs.h"

/*** C'est ici que se fait la conversion en mm 
 * ***/

// Roues 60 mm de diamètre, 188,5 mm de circonférence
// Réduction Moteur 30:1
// Réduction poulie 16:12
// Nombre d'impulsions par tour moteur : 200
// Nombre d'impulsions par tour réducteur : 6000
// Nombre d'impulsions par tour de roue : 8000
// Impulsion / mm : 42,44

#define IMPULSION_PAR_MM (42.44f)
#define ASSERMOTEUR_GAIN_P 160
#define ASSERMOTEUR_GAIN_I .80f

double consigne_mm_s[3]; // Consigne de vitesse (en mm/s)
double commande_I[3]; // Terme integral

void AsserMoteur_Init(){
    for(unsigned int i =0; i< 3; i ++){
        commande_I[i]=0;
        consigne_mm_s[i]=0;
    }
}


void AsserMoteur_setConsigne_mm_s(enum t_moteur moteur, double _consigne_mm_s){
    consigne_mm_s[moteur] = _consigne_mm_s;

}

double AsserMoteur_getVitesse_mm_s(enum t_moteur moteur, int step_ms){
    enum QEI_name_t qei;
    double distance, temps;
    switch (moteur)
    {
    case MOTEUR_A: qei = QEI_A_NAME; break;
    case MOTEUR_B: qei = QEI_B_NAME; break;
    case MOTEUR_C: qei = QEI_C_NAME; break;
    
    default: break;
    }
    distance = QEI_get_mm(qei);
    temps = step_ms / 1000.0;

    return distance / temps;
}

void AsserMoteur_Gestion(int step_ms){
    // Pour chaque moteur
    for(uint moteur=MOTEUR_A; moteur<MOTEUR_C+1; moteur++ ){
        double erreur; // Erreur entre la consigne et la vitesse actuelle
        double commande_P; // Terme proportionnel
        double commande;
        
        // Calcul de l'erreur
        erreur = consigne_mm_s[moteur] - AsserMoteur_getVitesse_mm_s(moteur, step_ms);

        // Calcul du terme propotionnel
        commande_P = erreur * ASSERMOTEUR_GAIN_P;

        // Calcul du terme integral
        commande_I[moteur] = commande_I[moteur] + (erreur * ASSERMOTEUR_GAIN_I * step_ms);

        commande = commande_P + commande_I[moteur];

        //Saturation de la commande
        if(commande > 32760) {commande = 32760;}
        if(commande < -32760) {commande = -32760;}

        Moteur_SetVitesse(moteur, commande);

    }
}