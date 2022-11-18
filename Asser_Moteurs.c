#include "QEI.h"
#include "Moteurs.h"

/*** C'est ici que ce fait la conversion en mm 
 * ***/

// Roues 60 mm de diamètre, 188,5 mm de circonférence
// Réduction Moteur 30:1
// Réduction poulie 16:12
// Nombre d'impulsions par tour moteur : 200
// Nombre d'impulsions par tour réducteur : 6000
// Nombre d'impulsions par tour de roue : 8000
// Impulsion / mm : 42,44

#define IMPULSION_PAR_MM (42.44f)
#define ASSERMOTEUR_GAIN_P 20
#define ASSERMOTEUR_GAIN_I 0.0f

double consigne_mm_s[3]; // Consigne de vitesse (en mm/s)
double commande_I[3]; // Terme integral



void AsserMoteur_setConsigne_mm_s(enum moteur_t moteur, double _consigne_mm_s){
    consigne_mm_s[moteur] = _consigne_mm_s;

}

double AsserMoteur_getVitesse_mm_s(enum moteur_t moteur){
    return (double) QEI_get(moteur) * (double)IMPULSION_PAR_MM;
}

AsserMoteur_Gestion(int step_ms){
    // Pour chaque moteur
    for(uint moteur=MOTEUR_A, i<MOTEUR_C+1; i++ ){
        double erreur; // Erreur entre la consigne et la vitesse actuelle
        double commande_P; // Terme proportionnel
        double commande;
        
        // Calcul de l'erreur
        erreur = consigne_mm_s - AsserMoteur_getVitesse_mm_s(moteur);

        // Calcul du terme propotionnel
        commande_P = erreur * ASSERMOTEUR_GAIN_P;

        // Calcul du terme integral
        commande_I[moteur] = commande_I[moteur] + (erreur * ASSERMOTEUR_GAIN_I * step_ms);

        commande = commande_P + commande_I;

        Moteur_SetVitesse(moteur, commande);

    }
}