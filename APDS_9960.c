#include "i2c_maitre.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define APDS_ADRESSE 0x39


inline void cc3_rgb2tls (uint8_t rouge, uint8_t vert, uint8_t bleu);

void APDS9960_Init(){
    char registre;
    char valeur;

    i2c_maitre_init();
    // Désactivation du module pour le configurer
    registre = 0x80;
    valeur = 0x0;
    while(i2c_ecrire_registre(APDS_ADRESSE, registre, valeur) != I2C_SUCCES);


    // Registres à configurer 
    // ATIME (0x81): temps d'acquisition 
    // | Valeur | Nb cycles |  Temps  |
    // |      0 |       255 |  712 ms |
    // |    182 |        72 |  200 ms |
    // |    255 |         1 | 2,78 ms |
    registre = 0x81;
    valeur = 220;
    while(i2c_ecrire_registre(APDS_ADRESSE, registre, valeur) != I2C_SUCCES);
    // CONTROL<AGAIN> (0x8F<1:0>): Gain
    // 0 : x1
    // 1 : x4
    // 2 : x16
    // 3 : x64
    registre = 0x8F;
    valeur = 0x03;
    while(i2c_ecrire_registre(APDS_ADRESSE, registre, valeur) != I2C_SUCCES);

    // Temps d'attente : Wen, WTIME, WLONG


    // ENABLE (PON) Address : 0x80 - Après les autres registres !!
    // RESERVED | Gesture En | Prox. Interrupt En | ALS Interrupt En | Wait En | Proximity Detect En |  ALS En | Power On |
    // Ob0000 0011 => 0x03
    registre = 0x80;
    valeur = 0x3;
    while(i2c_ecrire_registre(APDS_ADRESSE, registre, valeur) != I2C_SUCCES);

}

int APDS9960_Lire(){
    unsigned char reception[8];
    uint16_t clear, rouge, vert, bleu;
    i2c_lire_registre(APDS_ADRESSE, 0x94, reception, 8);
    clear = (uint16_t) reception[0] + (uint16_t) reception[1] << 8;
    rouge = (uint16_t) reception[2] + (uint16_t) reception[3] << 8;
    vert = (uint16_t) reception[4] + (uint16_t) reception[5] << 8;
    bleu = (uint16_t) reception[6] + (uint16_t) reception[7] << 8;


    printf("clear:%u\n", clear/256);
    printf("rouge:%u\n", rouge/256);
    printf("vert:%u\n", vert/256);
    printf("bleu:%u\n", bleu/256);


    cc3_rgb2tls(rouge/256, vert/256, bleu/256);

    return 1;

}

inline void cc3_rgb2tls (uint8_t rouge, uint8_t vert, uint8_t bleu)
{
  uint8_t hue, sat, val;
  uint8_t rgb_min, rgb_max;
  rgb_max = rouge;
  rgb_min = rouge;
  if (rouge > rgb_max)
    rgb_max = rouge;
  if (vert > rgb_max)
    rgb_max = vert;
  if (bleu > rgb_max)
    rgb_max = bleu;
  if (rouge < rgb_min)
    rgb_min = rouge;
  if (vert < rgb_min)
    rgb_min = vert;
  if (bleu < rgb_min)
    rgb_min = bleu;

// compute V
  val = rgb_max;
  if (val == 0) {
    printf("val:0");
    hue = sat = 0;
    return;
  }

// compute S
  sat = 255 * (rgb_max - rgb_min) / val;
  if (sat == 0) {
    printf("sat:0");
    return;
  }

// compute H
  if (rgb_max == rouge) {
    hue =
      0 + 43 * (int16_t)(vert -
                bleu) / (rgb_max - rgb_min);
  }
  else if (rgb_max == vert) {
    hue =
      85 + 43 * (int16_t)(bleu -
                 rouge) / (rgb_max - rgb_min);
  }
  else {                        /* rgb_max == blue */

    hue =
      171 + 43 * (int16_t)(rouge -
                  vert) / (rgb_max - rgb_min);
  }

  printf("val:%u\n", val);
  printf("sat:%u\n", sat);
  printf("teinte:%u\n", hue*360/250);
}