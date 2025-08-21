#ifndef HEADER_H
#define HEADER_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define MAX_DQT 4
#define MAX_DHT 4

/* -------------------------------------on définit les différentes structures pour chaque marqueur ---------------------------------- */

typedef struct {
    uint16_t longueur_section; 
    uint8_t precision; 
    uint8_t iQ; 
    uint16_t coefficients_zigzag[64];
} DQT;

typedef struct {
    uint16_t longueur_section; 
    uint8_t precision;
    uint16_t hauteur;
    uint16_t largeur;
    uint8_t nb_composantes; 
    uint8_t i_C[3]; 
    uint8_t facteur_echantillonage_H[3]; 
    uint8_t facteur_echantillonage_V[3]; 
    uint8_t i_Q[3]; 
    uint16_t taille_composantes[3]; //contient la taille des blocs des MCU des composantes 
    uint16_t nb_blocs[3]; //nombre de blocs pour la composante considérée dans le MCU 
    uint32_t nb_total_blocs; 
} SOF;

typedef struct {
    uint16_t longueur_section; 
    uint8_t type;
    uint8_t indice;    
    uint8_t nombre_symboles;
    uint8_t nb_codes_longueur[16];
    uint8_t symboles[256];
} DHT;

typedef struct {
    uint16_t longueur_section; 
    uint8_t nb_composantes; 
    uint8_t i_C[3]; 
    uint8_t i_H_DC[3];
    uint8_t i_H_AC[3];
    uint8_t Ss; 
    uint8_t Se; 
    uint8_t A_h; 
    uint8_t A_l;
    size_t taille_donnees; // taille des données encodées dans SOS 
    uint8_t* donnees;      // contient les données encodées dans SOS 
} SOS;

/* Structure pour retourner un tableau de chaînes de bits et la nouvelle valeur de "prem" */ 
typedef struct {
    char **codes;          // tableau de chaînes binaires
    uint16_t len;          // nombre d'éléments dans le tableau   la
    uint32_t prem;         // nouvelle valeur de prem
} HuffRes;

/* Structure pour retourner sous forme de tableau les chemins des symboles dans l'abre de Huffman */

typedef struct {
    char** huffman_path;
} Huff_Path;

/* Structure permettant de parcourir un flux de données */
typedef struct {
    uint8_t *donnees;        // Buffer contenant les données JPEG
    size_t taille;           // Taille totale en octets
    size_t position_octet;   // Position actuelle dans le buffer (en octets)
    int position_bit;        // Position actuelle dans l'octet (0 à 7)
} FluxBits;

typedef struct{
    int16_t **blocs;        // blocs décodés pour une composante 
    int nb_blocs;           // nombre total de blocs pour la composante considérée 
    uint8_t *** mcus;
} Decodage; 


/* ---------------------------------------------- structures et paramètres --------------------------------------------------------- */

extern DHT tableau_des_DHT[2][MAX_DHT]; //tableau contenant les tableaux DHT de types AC ou DC (max 4 tableaux)
extern DQT tableau_des_DQT[MAX_DQT]; 
extern SOF sof; 
extern int nombre_tableaux_dqt; // permet de savoir le nombre de tableaux déjà apparus dans l'entête de JPEG. Essentiel pour construire tableau_des_DQT
extern int nombre_tableaux_dht[2]; 
extern Huff_Path Huffman_path[2][MAX_DHT]; 
extern SOS sos;
extern Decodage Composantes_Decode[MAX_DHT]; 


/* ---------------------------------------------- les différentes fonctions -------------------------------------------------------- */

/* permet d'afficher l'entête JPEG */
extern void affichage(); 

/* permet de lire l'entête JPEG et de stocker les informations importantes */
extern int lecture_entete(FILE * fichier_JPEG); 

/* permet de lire deux octets dans un fichier JPEG à partir de l'octet courant et de les réunir pour former un uint16_t */
extern uint16_t recuperer_2_octets(FILE * fichier); 

/* prend en paramètre le tableau de 16 bits indiquant le nb de codes de longueur i, et renvoie l'encodage associé */
extern char **Huffman(const uint8_t tab[16]); 

/* Fonction récursive : renvoie un tableau de codes binaires et met à jour prem */
extern HuffRes Huffman_rec(uint32_t etage, uint32_t m, uint32_t prem); 

/*
     Fonction qui renvoie la représentation binaire de n sur exactement "width" bits
     Retourne une chaîne allouée dynamiquement (libérer après usage)
*/
extern char *binaire(uint32_t n, uint32_t width); 

/*  stocke les paths des symboles des arbres DHT */
extern int stockage_huffman_path(); 

/*  lecture de SOS et stockage des données importantes */
extern int lecture_entete_SOS(FILE * fichier_JPEG); 

/* renvoie les données encodés de la section SOS */
extern int donnees_encode_SOS(FILE* fichier_JPEG);

/* permet de décoder le code encodé par huffman dans SOS en renvoyant un indice i nous indiquant que le symboles décodés est le i-ème */
extern int decoder_huffman_symbol_from_path(Huff_Path *huffman_table, int nb_symboles, FluxBits *flux, int type, int indice);

/* initialise un flux de bits et prend en argument un pointeur sur un buffer et la taille de ce dernier */ 
extern FluxBits creer_flux_bits(uint8_t *donnees, size_t taille); 

/* prend en argument un pointeur vers la structure FluxBits, pour le modifier ds la fct, et renvoie le bit à lire */
extern int lire_bit(FluxBits *flux); 

/* permet de lire plusieurs bits consécutifs */
uint32_t lire_n_bits(FluxBits *flux, int n); 

/* prends en paramètre la magnitude et l'indice de l'élément dans la magnitude et renvoie le décodage jpeg associé*/
extern int decodage_valeur(int magnitude, int bits); 

extern int decoder_donnees();

/* multiplie les composantes décodées par la matrice de quantification */
extern void quantification_inverse(uint16_t coeff_quant[64], int16_t composantes_decodees[64]);

/* transforme un vecteur 1x64 en matrice 8x8 */
extern int16_t ** zig_zag_inv(int16_t * vecteur_quant);

/* calcul l'inverse de la DCT coefficient par coefficient */
extern uint8_t ** iDCT(int16_t ** bloc_frec);

extern int taille_composante_MCU();

extern int nb_blocs_composante_MCU();

extern int nb_total_blocs_image(); 

extern int ecrire_blocs_par_mcu_dans_fichier(const char *nom_fichier, char * sortie_souhaitee); 

/*
    effectue sur chacun des blocs 8x8 des composantes Y, Cb et Cr :
    - la quantification inverse ;
    - la réoeganisation en matrice via zigzag inverse ; 
    - le calcul de la DCT inverse sur la matrice obtenue ;
    - et enfin le sur-échantillonnage de cette matrice sur les blocs de Cb et Cr.
*/
extern void reconstitution_des_mcus();

/* 
    permet d'allouer un pointeur de pointeur dont le type est générique (matrice).
    les valeurs dans la matrice sont initialisées à 0
 */
extern void ** allouer_matrice(uint32_t sizeof_coef, uint32_t ligne, uint32_t colonne);
#define ALLOUER_MATRICE(type, ligne, colonne)   ((type **)allouer_matrice(sizeof(type),ligne, colonne))

/* libère l'espace mémoire occupé par la matrice */
extern void free_matrice(void ** matrice,uint32_t ligne);
#define FREE_MATRICE(matrice, ligne)   free_matrice((void **)matrice, ligne)

/* Crée h*v (h*v égal à 1,2 ou 4) matrices à partir de la matrice pointer par la variable composante */
extern uint8_t *** up_sampling(uint8_t *** composante, uint8_t h,uint8_t v, uint8_t nb_bloc_init);

extern void YCrCb_to_RGB();

#endif