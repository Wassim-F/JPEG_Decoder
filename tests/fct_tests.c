#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/header.h"

DHT tableau_des_DHT[2][MAX_DHT]; 
DQT tableau_des_DQT[MAX_DQT]; 
SOF sof; 
int nombre_tableaux_dqt = 0; 
int nombre_tableaux_dht[2]; 
Huff_Path Huffman_path[2][MAX_DHT];
SOS sos; 
Decodage Composantes_Decode[MAX_DHT]; 



void affiche_liste_matrice(uint8_t *** liste, uint8_t taille)
{
    printf("\n");
    for (int i=0; i<taille; i++){
        for (int j=0; j<8;j++){
            for (int k =0; k<8; k++){
                printf("%02u ",liste[i][j][k]);
            }printf("\n");
        } 
        printf("\n");
    } 
    printf("\n");

    for (int i=0; i<taille; i++){
        FREE_MATRICE(liste[i],8);
    } 
}


int main() 
{
    
    int16_t hex_data[8][8] = {{0x7C, 0, 0xFEE4, 0, 0, 0, 0xFFEC, 0},
                            {0xFFFA, 0, 0xFEDB, 0, 0x6A, 0, 0xFF7D, 0},
                            {0xFEB3, 0, 0xFF8B, 0, 0x45, 0, 0x9A, 0},
                            {0xFF36, 0, 0xFFA0, 0, 0x19, 0, 4, 0},
                            {0x80, 0, 0x011C, 0, 0xFF, 0, 0x14, 0},
                            {0xFFA6, 0, 0x13, 0, 0x7D, 0, 0xFE1F, 0},
                            {0xFF76, 0, 0x1A, 0, 0xFF59, 0, 0x00F4, 0},
                            {0xDD, 0, 0xFFA8, 0, 0xFFB9, 0, 0xFF3C, 0}};

                            
    printf("*** Tests de la fonction iDCT ***\n");
    printf("\nRésultat iDCT sur la matrice nulle :\n");
    
    // tests allocation de matrice 

    int16_t ** freq = ALLOUER_MATRICE(int16_t,8,8);
    uint8_t ** spatial = iDCT(freq);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            printf("%u ", spatial[i][j]);
        }
        printf("\n");
    }
    FREE_MATRICE(spatial,8);

    printf("\nTest sur le mcu de invader.jpeg :\n");
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            freq[i][j] = hex_data[i][j];
            printf("%04x ", (uint16_t)hex_data[i][j]);
        }
        printf("\n");
    }

    // Calcul de la iDCT
    spatial = iDCT(freq);

    printf("\nRésultat iDCT :\n");
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            printf("%02x ", spatial[i][j]);
        }
        printf("\n");
    }
    FREE_MATRICE(freq,8);
    FREE_MATRICE(spatial,8);

    // test upsampling

    uint8_t data[8][8] ={{1, 2, 3, 4, 5, 6, 7, 8},
                            {9, 10, 11, 12, 13, 14, 15, 16},
                            {17, 18, 19, 20, 21, 22, 23, 24},
                            {25, 26, 27, 28, 29, 30, 31, 32},
                            {33, 34, 35, 36, 37, 38, 39, 40},
                            {41, 42, 43, 44, 45, 46, 47, 48},
                            {49, 50, 51, 52, 53, 54, 55, 56},
                            {57, 58, 59, 60, 61, 62, 63, 64}};

                            
    printf("\n\n*** Tests de la fonction up_sampling ***\n");
    printf("matrice initiale : \n\n");
    for (int j=0; j<8;j++){
        for (int k =0; k<8; k++){
            printf("%02u ",data[j][k]);
        }printf("\n");
    } 
    printf("\n");

    uint8_t ** donnees;
    uint8_t *** sortie;
    
    // pas de upsampling
    donnees = ALLOUER_MATRICE(uint8_t,8,8);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            donnees[i][j] = data[i][j];
        }
    }
    printf("\n** Pas de sous échantillonnage : \n");
    sortie = up_sampling(&donnees, 1, 1, 1);
    affiche_liste_matrice(sortie,1);


    // division en 2 matrice : horizontal ou vertical
    donnees = ALLOUER_MATRICE(uint8_t,8,8);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            donnees[i][j] = data[i][j];
        }
    }
    sortie = up_sampling(&donnees, 2, 1, 1); // horizontal
    printf("\n** Sous échantillonnage horizontal : \n");
    affiche_liste_matrice(sortie,2);
    free(sortie);

    donnees = ALLOUER_MATRICE(uint8_t,8,8);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            donnees[i][j] = data[i][j];
        }
    }
    sortie = up_sampling(&donnees, 1, 2, 1); // vertical
    printf("\n** Sous échantillonnage vertical : \n");
    affiche_liste_matrice(sortie,2);
    free(sortie);

    // division en 4 matrice : horizontal et vertical
    donnees = ALLOUER_MATRICE(uint8_t,8,8);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            donnees[i][j] = data[i][j];
        }
    }
    sortie = up_sampling(&donnees, 2, 2, 1);
    printf("\n** Sous échantillonnage horizontal et vertical : \n");
    affiche_liste_matrice(sortie,4);
    free(sortie);

    return EXIT_SUCCESS;
    
}