#define TRAJET_EN_COURS 0
#define TRAJET_TERMINE  1


void Trajet_init();
void Trajet_debut_trajectoire(struct trajectoire_t trajectoire);
int Trajet_avance(double temps_s);
struct position_t Trajet_get_consigne(void);
