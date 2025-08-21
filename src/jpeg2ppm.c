#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/header.h"
#include <math.h>


/* -------------- on crée les différentes structures et paramètres qui nous intéressent ----------------*/

DHT tableau_des_DHT[2][MAX_DHT]; //tableau contenant les tableaux DHT de types AC ou DC (max 4 tableaux)
DQT tableau_des_DQT[MAX_DQT]; 
SOF sof; 
int nombre_tableaux_dqt = 0; // permet de savoir le nombre de tableaux déjà apparus dans l'entête de JPEG. Essentiel pour construire tableau_des_DQT
int nombre_tableaux_dht[2]; 
Huff_Path Huffman_path[2][MAX_DHT];
SOS sos; 
Decodage Composantes_Decode[MAX_DHT]; 



int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage : %s fichier.jpeg\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *fichier_JPEG ;
    char * sortie_souhaitee = strtok(argv[1],"=");

    if (strcmp(sortie_souhaitee,"--outfile") == 0) {
        fichier_JPEG = fopen(argv[2], "rb");
        sortie_souhaitee = strtok(NULL,"=");
    } else {
        fichier_JPEG = fopen(argv[1], "rb");
        sortie_souhaitee = NULL;
    }

    if (!fichier_JPEG) {
        fprintf(stderr, "Erreur : impossible d’ouvrir %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    lecture_entete(fichier_JPEG); 
    stockage_huffman_path(); 
    lecture_entete_SOS(fichier_JPEG);
    donnees_encode_SOS(fichier_JPEG);
    taille_composante_MCU();
    nb_blocs_composante_MCU();
    nb_total_blocs_image();
    decoder_donnees();
    reconstitution_des_mcus();
    
    //affichage();
    ecrire_blocs_par_mcu_dans_fichier(argv[1], sortie_souhaitee);
    free(sos.donnees);
    fclose(fichier_JPEG);
    return EXIT_SUCCESS;
}

