#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../include/header.h"
#include <math.h>

// constantes 
uint8_t zigzag_coord[128] = {0, 0, 0, 1, 1, 0, 2, 0, 1, 1, 0, 2, 0, 3, 1, 2, 
                             2, 1, 3, 0, 4, 0, 3, 1, 2, 2, 1, 3, 0, 4, 0, 5, 
                             1, 4, 2, 3, 3, 2, 4, 1, 5, 0, 6, 0, 5, 1, 4, 2, 
                             3, 3, 2, 4, 1, 5, 0, 6, 0, 7, 1, 6, 2, 5, 3, 4, 
                             4, 3, 5, 2, 6, 1, 7, 0, 7, 1, 6, 2, 5, 3, 4, 4, 
                             3, 5, 2, 6, 1, 7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 
                             7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 7, 4, 7, 5, 6, 
                             6, 5, 7, 4, 7, 5, 6, 6, 5, 7, 6, 7, 7, 6, 7, 7} ;
                             
double cosinus[8][8] ={ { 1.00000, 0.980785, 0.923880, 0.831470, 0.707107, 0.555570, 0.382683, 0.195090 },
                        { 1.00000, 0.831470, 0.382683, -0.195090, -0.707107, -0.980785, -0.923880, -0.555570 },
                        { 1.00000, 0.555570, -0.382683, -0.980785, -0.707107, 0.195090, 0.923880, 0.831470 },
                        { 1.00000, 0.195090, -0.923880, -0.555570, 0.707107, 0.831470, -0.382683, -0.980785 },
                        { 1.00000, -0.195090, -0.923880, 0.555570, 0.707107, -0.831470, -0.382683, 0.980785 },
                        { 1.00000, -0.555570, -0.382683, 0.980785, -0.707107, -0.195090, 0.923880, -0.831470 },
                        { 1.00000, -0.831470, 0.382683, 0.195090, -0.707107, 0.980785, -0.923880, 0.555570 },
                        { 1.00000, -0.980785, 0.923880, -0.831470, 0.707107, -0.555570, 0.382683, -0.195090 } };

     

/* -------- On code ici les fonctions qui vont servir à décoder le JPEG suivant le marqueur courant ------ */

/* ------------------------------- */

void affichage() {
    printf("=== Segment SOF ===\n");
    printf("Longueur section : %u\n", sof.longueur_section);
    printf("Précision : %u bits\n", sof.precision);
    printf("Dimensions : %u x %u\n", sof.largeur, sof.hauteur);
    printf("Nombre de composantes : %u\n", sof.nb_composantes);

    for (int i = 0; i < sof.nb_composantes; i++) {
        printf("Composante %d :\n", i);
        printf("  Identifiant (iC) : %u\n", sof.i_C[i]);
        printf("  Facteur hauteur : %u\n", sof.facteur_echantillonage_H[i]);
        printf("  Facteur V : %u\n", sof.facteur_echantillonage_V[i]);
        printf("  Indice Q : %u\n", sof.i_Q[i]);
    }

    printf("\n=== Segment DQT ===\n");
    for (int i = 0; i < nombre_tableaux_dqt; i++) {
        printf("Tableau DQT %d :\n", i);
        printf("  Longueur section : %u\n", tableau_des_DQT[i].longueur_section);
        printf("  Précision : %u\n", tableau_des_DQT[i].precision);
        printf("  Indice Q : %u\n", tableau_des_DQT[i].iQ);
        printf("  Coefficients :\n    ");
        for (int j = 0; j < 64; j++) {
            printf("%u ", tableau_des_DQT[i].coefficients_zigzag[j]);
            if ((j + 1) % 8 == 0) printf("\n    ");
        }
    }

    printf("\n=== Segment DHT ===\n");
    for (int type = 0; type < 2; type++) {
        const char *type_str = (type == 0) ? "DC" : "AC";
        for (int i = 0; i < nombre_tableaux_dht[type]; i++) {
            printf("Tableau DHT %d (%s) :\n", i, type_str);
            printf("  Longueur section : %u\n", tableau_des_DHT[type][i].longueur_section);
            printf("  Indice : %u\n", tableau_des_DHT[type][i].indice);
            printf("  Nb de codes par longueur : ");
            for (int j = 0; j < 16; j++) {
                printf("%u ", tableau_des_DHT[type][i].nb_codes_longueur[j]);
            }
            printf("\n  Symboles : ");
            for (int j = 0; j < tableau_des_DHT[type][i].nombre_symboles; j++) {
                printf("%02X ", tableau_des_DHT[type][i].symboles[j]);
            }
            printf("\n");
        }
    }

    printf("\n=== Chemins Huffman ===\n");
    for (int type = 0; type < 2; type++) {
        const char *type_str = (type == 0) ? "DC" : "AC";
        for (int i = 0; i < nombre_tableaux_dht[type]; i++) {
            printf("Chemins Huffman pour %s %d :\n", type_str, i);
            int nb = tableau_des_DHT[type][i].nombre_symboles;
            for (int j = 0; j < nb; j++) {
                printf("  Symbole 0x%02X : %s\n", tableau_des_DHT[type][i].symboles[j], Huffman_path[type][i].huffman_path[j]);
            }
        }
    }

    printf("\n=== Segment SOS ===\n");
    printf("Longueur section : %u\n", sos.longueur_section);
    printf("Nombre de composantes : %u\n", sos.nb_composantes);
    for (int i = 0; i < sos.nb_composantes; i++) {
        printf("Composante %d : identifiant = %u, table DC = %u, table AC = %u\n",
               i, sos.i_C[i], sos.i_H_DC[i], sos.i_H_AC[i]);
    }
    printf("Sélection spectrale : Ss = %u, Se = %u\n", sos.Ss, sos.Se);
    printf("Approximation successive : Ah = %u, Al = %u\n", sos.A_h, sos.A_l);

    printf("\n=== Données JPEG décodées ===\n");

    for (int comp = 0; comp < sof.nb_composantes; comp++) {
        printf("\nComposante %d :\n", comp);

       
        
        for (int bloc = 0; bloc < Composantes_Decode[comp].nb_blocs; bloc++) {
            printf("  Bloc %d :\n    ", bloc);

            for (int i = 0; i < 64; i++) {
                printf("%04X ", (uint16_t)(Composantes_Decode[comp].blocs[bloc][i]));
                if ((i + 1) % 8 == 0) printf("\n    ");
            }

            printf("\n");
        }
    }

    printf("\n=== MCUs reconstitués ===\n");
    reconstitution_des_mcus();
    for (int comp = 0; comp < sof.nb_composantes; comp++) {
        printf("\nComposante %d :\n", comp);

       
        
        for (int bloc = 0; bloc < Composantes_Decode[comp].nb_blocs; bloc++) {
            printf("  Bloc %d :\n    ", bloc);

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    printf("%02x ", (uint16_t)(Composantes_Decode[comp].mcus[bloc][i][j]));
                }
                printf("\n    ");
            }

            printf("\n");
        }
    }

}


   



/* ------------------------------- */

int lecture_entete(FILE * fichier_JPEG){
    /* on initialise octet qui représente l'octet courant et marqueur le marqueur courant */
    uint8_t octet; 
    uint8_t marqueur; 

    /* on commence la lecture */
    fread(&octet, 1, 1, fichier_JPEG); 
    fread(&marqueur, 1, 1, fichier_JPEG); 
    /* octet est initialisé avec le premier octet du fichier et marqueur avec le second*/
    if(octet != 0xFF || marqueur != 0xD8){
        /* cela veut dire que SOI n'est pas au début du fichier */
        fprintf(stderr, "Fichier JPEG invalide (pas de SOI)\n");
        fclose(fichier_JPEG);
        return EXIT_FAILURE;
    }
    
    /* si tout va bien, on continue la lecture du fichier */
    while(fread(&octet, 1, 1, fichier_JPEG) == 1){
        if(octet == 0xff){
            /* on repère que l'octet est 0xff donc potentiellement le début d'un marqueur 
            on doit donc vérifier les deux prochains octets au cas par cas  
            */
            fread(&marqueur, 1, 1, fichier_JPEG); 
            if((marqueur >= 0xE0 && marqueur <= 0xFD ) || marqueur == 0xFE){
                /* on rentre dans APPx ou COM qui nous intéressent pas, on veut sauter les octets */
                uint16_t taille = recuperer_2_octets(fichier_JPEG); 
                /* on saute alors les octets de la section en cours tout en oubliant pas 
                   que l'on a déjà parcouru deux octets 
                */
                fseek(fichier_JPEG, taille-2, SEEK_CUR); 
            }
            else if(marqueur == 0xc4){
                /* on rentre alors dans la section DHT */
                /* on récupère d'abord les deux octets définissant la taille de la section */
                int taille = recuperer_2_octets(fichier_JPEG); 
                /* on récupère ensuite le prochain octet*/
                fread(&octet, 1, 1, fichier_JPEG); 
                /* on récupère le type de DHT avec le 4 ième bit de octet */
                int type = (octet >> 4) & 0x01; 
                int compteur = nombre_tableaux_dht[type];
                /* maintenant que l'on sait le type on stocke tout*/
                tableau_des_DHT[type][compteur].longueur_section = taille; 
                tableau_des_DHT[type][compteur].type = (octet >> 4) & 0x01; 
                /* on récupère les 4 derniers bits qui correspondent à l'indice */
                tableau_des_DHT[type][compteur].indice = octet & 0x0F;
                /* ensuite on doit récupérer 16 bits pour les longueurs des codes */
                for(int j=0; j<16; j++){
                    fread(&octet, 1, 1, fichier_JPEG);
                    tableau_des_DHT[type][compteur].nb_codes_longueur[j] = octet; 
                }
                /* ensuite on doit récupérer les symboles triés en nombre taille_section - 2 -1-16*/
                int m = tableau_des_DHT[type][compteur].longueur_section - 19; 
                for(int j=0; j<m; j++){
                    fread(&octet, 1, 1, fichier_JPEG);
                    tableau_des_DHT[type][compteur].symboles[j] = octet; 
                }
                /* on ajoute le nombre de symbole */
                /* d'abord on initialise à 0 */
                tableau_des_DHT[type][compteur].nombre_symboles = 0; 
                for(int i=0; i<16; i++){
                    tableau_des_DHT[type][compteur].nombre_symboles += tableau_des_DHT[type][compteur].nb_codes_longueur[i]; 
                }
                nombre_tableaux_dht[type]++;
            }
            else if(marqueur == 0xDB){
                /* on rentre alors dans la section DQT */
                /* on récupère d'abord les deux octets définissant la taille de la section */
                tableau_des_DQT[nombre_tableaux_dqt].longueur_section = recuperer_2_octets(fichier_JPEG); 
                /* ensuite on doit récupérer la précision et l'indice dans l'octet suivant */
                fread(&octet, 1, 1, fichier_JPEG); 
                tableau_des_DQT[nombre_tableaux_dqt].precision = (octet >> 4); 
                tableau_des_DQT[nombre_tableaux_dqt].iQ = octet & 0x0F;
                /* ensuite on récupère les 64 bits des valeurs de la table de quantification */
                for(int j=0; j<64; j++){
                    fread(&octet, 1, 1, fichier_JPEG); 
                    tableau_des_DQT[nombre_tableaux_dqt].coefficients_zigzag[j] = octet; 
                }
                nombre_tableaux_dqt++ ; 
            }
            else if(marqueur == 0xC0){
                /* on rentre alors dans la section SOF */
                /* on récupère d'abord les deux octets définissant la taille de la section */
                sof.longueur_section = recuperer_2_octets(fichier_JPEG); 
                /* ensuite on récupère la précision en bit par composante */
                fread(&octet, 1, 1, fichier_JPEG); 
                sof.precision = octet; 
                /* ensuite on récupère hauteur */
                sof.hauteur = recuperer_2_octets(fichier_JPEG);
                /* puis largeur */
                sof.largeur = recuperer_2_octets(fichier_JPEG);
                /* puis le nombre de composantes */
                fread(&octet, 1, 1, fichier_JPEG); 
                sof.nb_composantes = octet; 
                /* puis on récupère les identifiants de composantes, les facteurs d'échantillonage et les iQ*/
                for(int N=0; N < sof.nb_composantes; N++){
                    fread(&octet, 1, 1, fichier_JPEG); 
                    /*on ajoute iC */
                    sof.i_C[N] = octet; 
                    /*on ajoute le facteur d'échantillonage */
                    fread(&octet, 1, 1, fichier_JPEG); 
                    sof.facteur_echantillonage_H[N] = (octet >> 4); 
                    sof.facteur_echantillonage_V[N] = octet & 0x0F;
                    /* enfin on ajoute iQ */
                    fread(&octet, 1, 1, fichier_JPEG); 
                    sof.i_Q[N] = octet; 
                }
            }
            else if(marqueur == 0xDA){
                /* on entre dans SOS donc fin d'entête donc on peut sortir de la fonction*/
                return EXIT_SUCCESS;
            }
            else{
                return EXIT_FAILURE;
            } 
        }
    }

    return EXIT_FAILURE;
}

/* ------------------------------- */

uint16_t recuperer_2_octets(FILE * fichier){
    uint8_t octet; 
    fread(&octet, 1, 1, fichier); 
    uint8_t taille_bit_poids_fort = octet; 
    fread(&octet, 1, 1, fichier); 
    uint8_t taille_bit_poids_faible = octet; 
    uint16_t taille = (taille_bit_poids_fort << 8) | taille_bit_poids_faible; 
    return taille; 
} 

/* ------------------------------- */

// Fonction qui renvoie la représentation binaire de n sur exactement "width" bits
// Retourne une chaîne allouée dynamiquement (libérer après usage)
char *binaire(uint32_t n, uint32_t width) {
    char *buf = malloc(width + 1);
    if (!buf) {
        fprintf(stderr, "Erreur d'allocation\n");
        exit(EXIT_FAILURE);
    }
    buf[width] = '\0';
    for (int i = width - 1; i >= 0; --i) {
        buf[i] = (n & 1) ? '1' : '0';
        n >>= 1;
    }
    return buf;
}

/* ------------------------------- */

// Fonction récursive : renvoie un tableau de codes binaires et met à jour prem
HuffRes Huffman_rec(uint32_t etage, uint32_t m, uint32_t prem) {
    HuffRes res;
    if (etage == 1) {
        if (m == 0) {
            // tableau vide
            res.len = 0;
            res.codes = NULL;
            res.prem = 1;
            return res;
        } else if (m == 1) {
            // un seul bit 0
            res.len = 1;
            res.codes = malloc(sizeof(char*));
            if (!res.codes) { fprintf(stderr, "Erreur d'allocation\n"); exit(EXIT_FAILURE); }
            res.codes[0] = malloc(2);
            if (!res.codes[0]) { fprintf(stderr, "Erreur d'allocation\n"); exit(EXIT_FAILURE); }
            res.codes[0][0] = '0';
            res.codes[0][1] = '\0';
            res.prem = 3;
            return res;
        } else if (m == 2) {
            // deux bits 0 et 1
            res.len = 2;
            res.codes = malloc(2 * sizeof(char*));
            if (!res.codes) { fprintf(stderr, "Erreur d'allocation\n"); exit(EXIT_FAILURE); }
            // code "0"
            res.codes[0] = malloc(2);
            if (!res.codes[0]) { fprintf(stderr, "Erreur d'allocation\n"); exit(EXIT_FAILURE); }
            res.codes[0][0] = '0';
            res.codes[0][1] = '\0';
            // code "1"
            res.codes[1] = malloc(2);
            if (!res.codes[1]) { fprintf(stderr, "Erreur d'allocation\n"); exit(EXIT_FAILURE); }
            res.codes[1][0] = '1';
            res.codes[1][1] = '\0';
            res.prem = prem;
            return res;
        }
        // cas par défaut si m > 2
        res.len = 0;
        res.codes = NULL;
        res.prem = prem;
        return res;
    }
    // Cas général : tableau de longueur m
    res.len = m;
    res.codes = malloc(res.len * sizeof(char*));
    if (!res.codes) {
        fprintf(stderr, "Erreur d'allocation\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < m; i++) {
        res.codes[i] = binaire(i + prem-1, etage);
    }
    res.prem = (prem - 1) * 2 + 2 * m + 1;
    return res;
}

/* ------------------------------- */

// Fonction principale : parcourt "len" symboles de tab et affiche chaque tableau sur une ligne
char **Huffman(const uint8_t tab[16]) {
    uint32_t prem = 1;
    int16_t capacity = 64; //la
    uint16_t total = 0;   //la
    char **all_codes = malloc(capacity * sizeof(char*));
    if (!all_codes) {
        fprintf(stderr, "Erreur d'allocation\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < 16; ++i) { //la
        uint32_t etage = i + 1;
        uint32_t m = tab[i];
        if (m == 0){
            prem = 2*(prem-1)+1;
            continue;
        }

        HuffRes r = Huffman_rec(etage, m, prem);

        if (r.len > 0) {
            if (total + r.len > capacity) {
                capacity = (total + r.len) * 2;
                all_codes = realloc(all_codes, capacity * sizeof(char*));
                if (!all_codes) {
                    fprintf(stderr, "Erreur de realloc\n");
                    exit(EXIT_FAILURE);
                }
            }
            for (size_t j = 0; j < r.len; ++j) {
                all_codes[total++] = r.codes[j];
            }
        }

        prem = r.prem;
        free(r.codes);
    }

    return all_codes;  // Je connais la taille = somme(tab[0] à tab[15])
}

/* ------------------------------- */

int stockage_huffman_path() {
    for (int type = 0; type < 2; type++) {
        for (int i = 0; i < nombre_tableaux_dht[type]; i++) {
            char **codes = Huffman(tableau_des_DHT[type][i].nb_codes_longueur);
            Huffman_path[type][i].huffman_path = codes;
        }
    }
    return EXIT_SUCCESS;
}

/* ------------------------------------------------ */

int lecture_entete_SOS(FILE * fichier_JPEG){
    /* on initialise octet qui représente l'octet courant */
    uint8_t octet; 


    /* on commence la lecture. La fonction lecture entete utilisé avant dans le main va amener 
       l'octet courant à 0xDA donc la fin de marqueur de SOS  
    */

    /* on lit la taille de la section */
    sos.longueur_section = recuperer_2_octets(fichier_JPEG);     

    /* on récupère le nombre de composantes */
    fread(&octet, 1, 1, fichier_JPEG); 
    sos.nb_composantes = octet; 

    for(int i=0; i<sos.nb_composantes; i++){
        fread(&octet, 1, 1, fichier_JPEG); 
        sos.i_C[i] = octet; 
        fread(&octet, 1, 1, fichier_JPEG); 
        sos.i_H_DC[i] = (octet >> 4); 
        sos.i_H_AC[i] = octet & 0x0F; 
    }
    fread(&octet, 1, 1, fichier_JPEG); 
    sos.Ss = octet; 
    fread(&octet, 1, 1, fichier_JPEG); 
    sos.Se = octet; 

    fread(&octet, 1, 1, fichier_JPEG); 
    sos.A_h = (octet >> 4); 
    sos.A_l = octet & 0x0F; 

    return EXIT_SUCCESS;
}

/* ------------------------------- */ 

/* renvoie les données encodés de la section SOS */
int donnees_encode_SOS(FILE* fichier_JPEG){
    uint8_t octet; 
    uint8_t marqueur; 
    size_t i = 0; 

    size_t capacite = 1024; 
    sos.donnees = malloc(capacite); 
    if (!sos.donnees) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        return EXIT_FAILURE;
    }

    while(fread(&octet, 1, 1, fichier_JPEG) == 1){
        if(octet == 0xff){
            fread(&marqueur, 1, 1, fichier_JPEG); 
            if(marqueur == 0xD9){
                break; 
            }
            else{
                /* on ajoute octet et marqueur en faisant attention au dépassement de mémoire */
                if(i+2 >= capacite){
                    capacite *= 2;
                    sos.donnees = realloc(sos.donnees, capacite);
                    if (!sos.donnees) {
                        fprintf(stderr, "Erreur realloc\n");
                        return EXIT_FAILURE;
                    }
                }
                sos.donnees[i++] = 0xFF;
                sos.donnees[i++] = marqueur;
            }
        }
        else {
                /* on ajoute octet en faisant attention au dépassement de mémoire */
                if (i >= capacite) {
                    capacite *= 2;
                    sos.donnees = realloc(sos.donnees, capacite);
                    if (!sos.donnees) {
                        fprintf(stderr, "Erreur realloc\n");
                        return EXIT_FAILURE;
                    }
                }
                sos.donnees[i++] = octet;
        }
           
    }
    sos.taille_donnees = i;  
    return EXIT_SUCCESS;
}

/* ------------------------------- */ 


int decoder_huffman_symbol_from_path(Huff_Path *huffman_table, int nb_symboles, FluxBits *flux, int type, int indice) {
    char code_lu[17];           // chaine temporaire pour stocker le code lu 
    int longueur_code = 0;      // nombre de bits actuellement lus 

    while (longueur_code < 16) {
        int bit_lu = lire_bit(flux); 

        if (bit_lu == -1) {
            return -1;
        }

        /* on convertit le bit en 1 ou 0 */
        if (bit_lu == 1) {
            code_lu[longueur_code] = '1';
        } else {
            code_lu[longueur_code] = '0';
        }

        longueur_code++;
        /* on termine la chaine avec \0 */
        code_lu[longueur_code] = '\0'; 

        /* compare avec tous les codes de Huffman */
        for (int i = 0; i < nb_symboles; i++) {
            if (strcmp(code_lu, huffman_table->huffman_path[i]) == 0) {
                return tableau_des_DHT[type][indice].symboles[i]; /* le symbole a été trouvé */
            }
        }
    }

    /* si aucun code trouvé après 16 bits on return -1 */
    return -1;
}

/* ------------------------------- */ 

/* initialise un flux de bits et prend en argument un pointeur sur un buffer et la taille de ce dernier */ 
FluxBits creer_flux_bits(uint8_t *donnees, size_t taille) {
    FluxBits flux;
    flux.donnees = donnees;
    flux.taille = taille;
    flux.position_octet = 0;
    flux.position_bit = 0;
    return flux;
}

/* ------------------------------- */ 

/* prend en argument un pointeur vers la structure FluxBits, pour le modifier ds la fct, et renvoie le bit à lire */
int lire_bit(FluxBits *flux) {
    if (flux->position_octet >= flux->taille) return -1;

    uint8_t octet = flux->donnees[flux->position_octet];
    int bit = (octet >> (7 - flux->position_bit)) & 1;

    flux->position_bit++;

    if (flux->position_bit == 8) {
        flux->position_bit = 0;
        flux->position_octet++;

        // Vérifie le byte stuffing : 0xFF suivi de 0x00
        if (octet == 0xFF &&
            flux->position_octet < flux->taille &&
            flux->donnees[flux->position_octet] == 0x00) {
            flux->position_octet++; // sauter le 0x00 après avoir fini le 0xFF
        }
    }

    return bit;
}


/* ------------------------------- */ 

/* permet de lire plusieurs bits consécutifs */
uint32_t lire_n_bits(FluxBits *flux, int n) {
    uint32_t resultat = 0;
    for (int i = 0; i < n; i++) {
        int bit = lire_bit(flux);
        if (bit == -1){
            break;
        }
        resultat = (resultat << 1) | bit;
    }
    return resultat;
}

/* ------------------------------- */  

/* prends en paramètre la magnitude et l'indice de l'élément dans la magnitude et renvoie le décodage jpeg associé*/
int decodage_valeur(int magnitude, int bits) {
    if (magnitude == 0) {
        return 0;
    }

    /* le seuil est la plus petite valeur positive à la magnitude considérée */
    int seuil = 1 << (magnitude - 1);  // 2^(m-1)
    /* min_magnitude est la valeur min de la magnitude étudiée */
    int min_magnitude = 1 - (1 << magnitude); //1-2^m 

    if (bits < seuil) {
        // Valeur négative
        return min_magnitude + bits;
    } else {
        // Valeur positive
        return bits;
    }
}

/* ------------------------------- */  

int taille_composante_MCU(){
    int H;
    int V; 
    for(int i=0; i<sof.nb_composantes; i++){
        H = 8*sof.facteur_echantillonage_H[i];
        V = 8*sof.facteur_echantillonage_V[i];
        sof.taille_composantes[i] = H*V;  
    }
    return EXIT_SUCCESS;
}

int nb_blocs_composante_MCU(){
    for(int i=0; i<sof.nb_composantes; i++){
        sof.nb_blocs[i] = sof.facteur_echantillonage_H[i]*sof.facteur_echantillonage_V[i];
    }
    return EXIT_SUCCESS;
}

int nb_total_blocs_image(){
    /* on rajoute 8 * sof.facteur_echantillonage_H[0] - 1 pour faire un arrondi supérieur */
    int nb_MCU_largeur = (sof.largeur + 8 * sof.facteur_echantillonage_H[0] - 1) / (8 * sof.facteur_echantillonage_H[0]);
    int nb_MCU_hauteur = (sof.hauteur + 8 * sof.facteur_echantillonage_V[0] - 1) / (8 * sof.facteur_echantillonage_V[0]);
    sof.nb_total_blocs = nb_MCU_hauteur * nb_MCU_largeur;
    return EXIT_SUCCESS; 
}     
int decoder_donnees() {
    FluxBits flux = creer_flux_bits(sos.donnees, sos.taille_donnees);

    /* Allocation mémoire pour chaque composante */
    for (int comp = 0; comp < sof.nb_composantes; comp++) {
        int nb_blocs_totaux = sof.nb_total_blocs * sof.nb_blocs[comp];
        Composantes_Decode[comp].nb_blocs = nb_blocs_totaux;
        Composantes_Decode[comp].blocs = ALLOUER_MATRICE(int16_t,nb_blocs_totaux,64);
    }

    /* parcours des MCU */
    int16_t precedent_DC[3] = {0};  /* pour faire le calcul par différence*/

    for (uint32_t mcu = 0; mcu < sof.nb_total_blocs; mcu++) {
        for (int comp = 0; comp < sof.nb_composantes; comp++) {
            int dc_index = sos.i_H_DC[comp];
            int ac_index = sos.i_H_AC[comp];

            int nb_sym_dc = tableau_des_DHT[0][dc_index].nombre_symboles;
            int nb_sym_ac = tableau_des_DHT[1][ac_index].nombre_symboles;

            for (int bloc = 0; bloc < sof.nb_blocs[comp]; bloc++) {
                int bloc_index = mcu * sof.nb_blocs[comp] + bloc;

                /* DC */
                int magnitude_DC = decoder_huffman_symbol_from_path(
                    &Huffman_path[0][dc_index], nb_sym_dc, &flux, 0, dc_index);

                uint32_t indice_DC = lire_n_bits(&flux, magnitude_DC);
                int diff_DC = decodage_valeur(magnitude_DC, indice_DC);
                int valeur_DC = precedent_DC[comp] + diff_DC;
                precedent_DC[comp] = valeur_DC;

                Composantes_Decode[comp].blocs[bloc_index][0] = valeur_DC;

                /* AC */
                int coeff_index = 1;
                while (coeff_index < 64) {
                    int symbole = decoder_huffman_symbol_from_path(
                        &Huffman_path[1][ac_index], nb_sym_ac, &flux, 1, ac_index);

                    if (symbole == 0x00) {
                        /* cas EOB */
                        while (coeff_index < 64) {
                            Composantes_Decode[comp].blocs[bloc_index][coeff_index] = 0;
                            coeff_index++;
                        }
                        break;
                    }

                    if (symbole == 0xF0) {
                        /* 16 zeros */
                        for (int z = 0; z < 16 ; z++) {
                            Composantes_Decode[comp].blocs[bloc_index][coeff_index] = 0;
                            coeff_index++;
                        }
                        continue;
                    }

                    int nb_zeros = symbole >> 4;
                    int magnitude_AC = symbole & 0x0F;

                    /* mettre les nb_zeros avant la valeur */
                    for (int z = 0; z < nb_zeros; z++) {
                        Composantes_Decode[comp].blocs[bloc_index][coeff_index] = 0;
                        coeff_index++;
                    }

                    uint32_t indice_AC = lire_n_bits(&flux, magnitude_AC);
                    int valeur_AC = decodage_valeur(magnitude_AC, indice_AC);
                    Composantes_Decode[comp].blocs[bloc_index][coeff_index] = valeur_AC;
                    coeff_index++;
                }
            }
        }
    }

    for (int type = 0; type < 2; type++) {
        for (int i = 0; i < nombre_tableaux_dht[type]; i++) {
            int nb = tableau_des_DHT[type][i].nombre_symboles;
            FREE_MATRICE(Huffman_path[type][i].huffman_path,nb);
        }
    }

    return EXIT_SUCCESS;
}


void ** allouer_matrice(uint32_t sizeof_coef, uint32_t ligne, uint32_t colonne)
{
    void ** matrice = malloc(ligne*sizeof(void *));
    for (uint32_t i=0; i<ligne; i++) { matrice[i] = calloc(colonne,sizeof_coef);}
    return matrice;
}

void free_matrice(void ** matrice, uint32_t ligne)
{

    for (uint32_t i=0; i<ligne; i++) { free(matrice[i]); }
    free(matrice);
    
}

void quantification_inverse(uint16_t coeff_quant[64], int16_t composantes_decodees[64])
{
    //int16_t * quant_inv = malloc(64*sizeof(uint16_t)); 
    for (uint8_t i=0; i<64; i++) {
        composantes_decodees[i] *= coeff_quant[i];
    }
}

int16_t ** zig_zag_inv(int16_t * vecteur) 
{
    int16_t ** table = ALLOUER_MATRICE(int16_t,8,8);

    uint8_t * c = zigzag_coord;
    for (uint8_t i = 0; i<64; i++) {
        table[c[2*i]][c[2*i+1]] = vecteur[i];
    }
    return table;
}

uint8_t coef_iDCT(int16_t ** bloc_frec, uint8_t x, uint8_t y)
{
    double sum = 0,Ci,Cj;

    for (uint8_t i = 0; i < 8; i++) {
        Ci = (i == 0) ? 1 / sqrt(2) : 1;

        for (uint8_t j = 0; j < 8; j++) {
            Cj = (j == 0) ? 1 / sqrt(2) : 1;
            sum += Ci * Cj * cosinus[x][i] * cosinus[y][j] * (double)bloc_frec[i][j];
        }
    }

    sum = sum/4 + 128;

    if (sum < 0) sum = 0;
    else if (sum > 255) sum = 255;

    return (uint8_t)round(sum);
}

uint8_t ** iDCT(int16_t ** bloc_frec) 
{
    uint8_t ** bloc_pixel = ALLOUER_MATRICE(uint8_t,8,8);

    for (uint8_t x=0; x<8; x++) {
        for (uint8_t y=0; y<8; y++) {
            bloc_pixel[x][y] = coef_iDCT(bloc_frec,x,y);
        }
    }
    return bloc_pixel;
}

void reconstitution_des_mcus()
{
    
    for (int comp = 0; comp < sof.nb_composantes; comp++) { 
        Composantes_Decode[comp].mcus = malloc(Composantes_Decode[0].nb_blocs*sizeof(uint8_t**)); 
    }

    for (uint32_t mcu = 0; mcu < sof.nb_total_blocs; mcu++) {
        for (int comp = 0; comp < sof.nb_composantes; comp++) {  
            for (int bloc = 0; bloc < sof.nb_blocs[comp]; bloc++) {
                int bloc_index = mcu * sof.nb_blocs[comp] + bloc;

                
                uint8_t iQ = sof.i_Q[comp];
                quantification_inverse(tableau_des_DQT[iQ].coefficients_zigzag,Composantes_Decode[comp].blocs[bloc_index]);
                int16_t ** bloc_freq = zig_zag_inv(Composantes_Decode[comp].blocs[bloc_index]);
                uint8_t ** bloc_pixel = iDCT(bloc_freq);
                
                FREE_MATRICE(bloc_freq,8);
                free(Composantes_Decode[comp].blocs[bloc_index]);

                if (comp > 0) {
                    uint8_t h = sof.facteur_echantillonage_H[0]/sof.facteur_echantillonage_H[comp];
                    uint8_t v = sof.facteur_echantillonage_V[0]/sof.facteur_echantillonage_V[comp];
                    uint8_t *** sortie = up_sampling(&bloc_pixel, h, v, sof.nb_blocs[comp]);
                    
                    int nb_elem = sof.facteur_echantillonage_H[0]*sof.facteur_echantillonage_V[0];
                    for (int i=0; i<nb_elem; i++) {
                        Composantes_Decode[comp].mcus[nb_elem*bloc_index+i] = sortie[i];
                    }
                    free(sortie);

                    Composantes_Decode[comp].nb_blocs = Composantes_Decode[0].nb_blocs;
                    
                } else { 
                    Composantes_Decode[comp].mcus[bloc_index] = bloc_pixel; 
                }
            }        
        } 
    }
    sof.nb_blocs[1]=sof.nb_blocs[0];sof.nb_blocs[2]=sof.nb_blocs[0];
    free(Composantes_Decode[0].blocs);
    free(Composantes_Decode[1].blocs);
    free(Composantes_Decode[2].blocs);
}

uint8_t *** up_sampling(uint8_t *** composante, uint8_t h,uint8_t v, uint8_t nb_bloc_init)
{

    uint8_t nb_bloc_sortie = h*v;
    uint8_t ** matrice = *composante;
    if (nb_bloc_init < nb_bloc_sortie) {
        uint8_t *** sortie = malloc(nb_bloc_sortie*sizeof(uint8_t**));
        for (uint8_t i=0; i<nb_bloc_sortie; i++) {
            sortie[i] = ALLOUER_MATRICE(uint8_t, 8,8);
        }

        if (nb_bloc_sortie == 4) {
            for (int bloc=0; bloc<2; bloc++) {

                int ligne_sortie = 0;
                for (int ligne_matrice = 4*bloc; ligne_matrice < 4*(bloc+1); ligne_matrice++) {

                    for (int coef = 0; coef < 4; coef++) {

                        sortie[2*bloc][ligne_sortie][2*coef] = matrice[ligne_matrice][coef];
                        sortie[2*bloc][ligne_sortie][2*coef+1] = matrice[ligne_matrice][coef];
                        sortie[2*bloc][ligne_sortie+1][2*coef] = matrice[ligne_matrice][coef];
                        sortie[2*bloc][ligne_sortie+1][2*coef+1] = matrice[ligne_matrice][coef];
                        
                        sortie[2*bloc+1][ligne_sortie][2*coef] = matrice[ligne_matrice][coef+4];
                        sortie[2*bloc+1][ligne_sortie][2*coef+1] = matrice[ligne_matrice][coef+4];
                        sortie[2*bloc+1][ligne_sortie+1][2*coef] = matrice[ligne_matrice][coef+4];
                        sortie[2*bloc+1][ligne_sortie+1][2*coef+1] = matrice[ligne_matrice][coef+4];
                    }
                    ligne_sortie+=2;
                }
            }
        } else if (nb_bloc_sortie == 2) {
            int ligne_sortie = 0;
            if (h < v) {
            
                for (int ligne_matrice = 0; ligne_matrice < 4; ligne_matrice++) {
                    for (int coef = 0; coef < 8; coef++) {
                        
                        sortie[0][ligne_sortie][coef] = matrice[ligne_matrice][coef];
                        sortie[0][ligne_sortie+1][coef] = matrice[ligne_matrice][coef];
                        
                        sortie[1][ligne_sortie][coef] = matrice[ligne_matrice+4][coef];
                        sortie[1][ligne_sortie+1][coef] = matrice[ligne_matrice+4][coef];
                    }
                    ligne_sortie+=2;
                }
            } else {

                for (int ligne_matrice = 0; ligne_matrice < 8; ligne_matrice++) {
                    
                    for (int coef = 0; coef < 4; coef++) {

                        sortie[0][ligne_matrice][2*coef] = matrice[ligne_matrice][coef];
                        sortie[0][ligne_matrice][2*coef+1] = matrice[ligne_matrice][coef];
                        
                        sortie[1][ligne_matrice][2*coef] = matrice[ligne_matrice][coef+4];
                        sortie[1][ligne_matrice][2*coef+1] = matrice[ligne_matrice][coef+4];  
                    }
                }
            }
        } else {
            for (uint8_t i=0; i<nb_bloc_sortie; i++) {
                FREE_MATRICE(sortie[i],8);
            } 
            free(sortie);
            return composante;
        }
        FREE_MATRICE(*composante,8);
        return sortie;
    } else { return composante;}
}

void YCrCb_to_RGB()
{
    int nb_blocs = Composantes_Decode[0].nb_blocs;

    for (int bloc = 0; bloc < nb_blocs; bloc++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int Y  = Composantes_Decode[0].mcus[bloc][i][j];
                int Cb = Composantes_Decode[1].mcus[bloc][i][j] - 128;
                int Cr = Composantes_Decode[2].mcus[bloc][i][j] - 128;

                int R = Y + 1.402 * Cr;
                int G = Y - 0.34414 * Cb - 0.71414 * Cr;
                int B = Y + 1.772 * Cb;

                if (R < 0) {R = 0;}; if (R > 255) {R = 255;};
                if (G < 0) {G = 0;}; if (G > 255) {G = 255;};
                if (B < 0) {B = 0;}; if (B > 255) {B = 255;};

                Composantes_Decode[0].mcus[bloc][i][j] = R;
                Composantes_Decode[1].mcus[bloc][i][j] = G;
                Composantes_Decode[2].mcus[bloc][i][j] = B;
            }
        }
    }
}




int ecrire_blocs_par_mcu_dans_fichier(const char *nom_fichier,char * sortie_souhaitee)
{
    char new_filename[256];
    char * extension = (sof.nb_composantes == 1) ? ".pgm" : ".ppm";
    char * marqueur  = (sof.nb_composantes == 1) ? "P5"   : "P6"; 

    if (sortie_souhaitee == NULL) {
        // Trouver la position du dernier point dans le nom de fichier
        const char *position = strrchr(nom_fichier, '.');
        size_t taille = position - nom_fichier;

        // Copier le nom de base dans new_filename
        strncpy(new_filename, nom_fichier, taille);
        new_filename[taille] = '\0'; 

        // Ajouter la nouvelle extension
        strcat(new_filename, extension);
    } else {strcpy(new_filename, sortie_souhaitee);}

    FILE *fichier = fopen(new_filename, "wb");
    if (fichier == NULL) {
        fprintf(stderr,"Erreur lors de l'ouverture du fichier");
        return EXIT_FAILURE;
    }
    fprintf(fichier, "%s\n%d %d\n255\n",marqueur, sof.largeur, sof.hauteur);

    
    if(sof.nb_composantes == 1){
        
        /* on ajoute 8 * sof.facteur_echantillonage_H[0] - 1 pour gérer la troncature  */
        int nb_MCU_largeur = (sof.largeur + 8 * sof.facteur_echantillonage_H[0] - 1) / (8 * sof.facteur_echantillonage_H[0]);
        /* on fait bien attention à lire seulement les pixels qui nous intéressent */
        
        for (int y = 0; y < sof.hauteur; y++) {
            int bloc_ligne = y / 8; 
            int ligne_dans_bloc = y % 8; 

            for (int x = 0; x < sof.largeur; x++) {
                int bloc_col = x / 8;
                int col_dans_bloc = x % 8;

                int bloc_index = bloc_ligne * nb_MCU_largeur + bloc_col;
                uint8_t pixel = Composantes_Decode[0].mcus[bloc_index][ligne_dans_bloc][col_dans_bloc];
                fwrite(&pixel, 1, 1, fichier);
            }
        }
        
    }
        
    else{
        
        YCrCb_to_RGB(); 

        int largeurMCU = 8 * sof.facteur_echantillonage_H[0];
        int hauteurMCU = 8 * sof.facteur_echantillonage_V[0];
        /* nombre de MCU par ligne et nombre total de MCU */
        int nb_MCU_largeur = (sof.largeur  + largeurMCU  - 1) / largeurMCU;
        /* nombre de blocs 8×8 dans chaque MCU */
        int blocsParMCU = sof.facteur_echantillonage_H[0] * sof.facteur_echantillonage_V[0];

        for (int y = 0; y < sof.hauteur; y++) {
            /* coordonnées MCU */
            int mcuY = y / hauteurMCU;
            int dy   = y % hauteurMCU;
            /* dans quel bloc vertical de la composante on se trouve */
            int bloc_ligne = dy / 8;
            /* décalage à l’intérieur du bloc */
            int ligne_dans_bloc = dy % 8;

            for (int x = 0; x < sof.largeur; x++) {
                int mcuX = x / largeurMCU;
                int dx   = x % largeurMCU;
                int bloc_col = dx / 8;
                int col_dans_bloc = dx % 8;

                /* indice du MCU dans l’ordre lecture */
                int mcu_index = mcuY * nb_MCU_largeur + mcuX;
                /* indice du bloc 8×8 dans la MCU */
                int idxBloc = mcu_index * blocsParMCU
                            + bloc_ligne * sof.facteur_echantillonage_H[0]
                            + bloc_col;

                uint8_t R = Composantes_Decode[0].mcus[idxBloc][ligne_dans_bloc][col_dans_bloc];
                uint8_t G = Composantes_Decode[1].mcus[idxBloc][ligne_dans_bloc][col_dans_bloc];
                uint8_t B = Composantes_Decode[2].mcus[idxBloc][ligne_dans_bloc][col_dans_bloc];
                
                fwrite(&R, 1, 1, fichier); 
                fwrite(&G, 1, 1, fichier);
                fwrite(&B, 1, 1, fichier);
            }
        }

    }

    for (int comp=0; comp <sof.nb_composantes; comp++){
        for (int i = 0; i<Composantes_Decode[0].nb_blocs; i++) {
            FREE_MATRICE(Composantes_Decode[comp].mcus[i],8);
        }
        free(Composantes_Decode[comp].mcus);
    }
    
    fclose(fichier);
    return EXIT_SUCCESS;
}
