#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

/* Fonctions exécutées par les threads */
void *runner_ligne(void *param);
void *runner_colonne(void *param);
void *runner_bloc(void *param);
void *runner_taille9x9(void *param);
void *runner_caractere(void *param); 

//Tailles constantes utilisées a divers endroit
#define NB_SUDOKU 4
#define NB_COLONNES_TOTAL 36 // 9 * 4 || 9 (ligne-colonnes-bloc) * 4(sudoku)
#define TAILLE_COLONNE 9 //Mais aussi de ligne et le nombre de sous-grille
#define TAILLE_BLOC 3
#define ESPACE -1	//Pour gérer un espace dans un traitement de int
#define SPECIAL -2	//idem -> pour les traîter en int
#define CARACTERE -3 //idem -> pour les traîter en int
#define FIN_LECTURE -4 //décrit le surplus dans le tableau par rapport aux entrées en lecture
#define TAILLE_LECTURE 10 //décrit le nombre qu'on lit en lecture


//Pour gérer les booléens facilement 
//Soit déclaré avec 'bool' et prenant les valeurs 'true' et 'false'
typedef enum bool bool;
enum bool
{
true = 1, false = 0
};

//Structure définissant les paramètres envoyés aux threads
typedef struct
{
	int ligne;
	int colonne;
	int numSudoku; //définie le numéro du sudoku traité
} Parameters;

//Structure définissant un case avec :
typedef struct
{
	int l; //ligne
	int c; //colonne
	int v; //valeur
}Doublon; //Appeler Doublon car utilisé pour les trouver

//Structure définissant un doublon pour une ligne / une colonne / une sous-grille
typedef struct
{
	bool trouver; //Doublon déjà trouvé ? true or false
	int valeur; //valeur du doublons
	int ligne1;	//Les positions du doublon
	int colonne1;
	int ligne2;
	int colonne2;
}DoublonResultat;

//Tableau déterminant si un sudoku est toujours bon où non
//Pour savoir s'il faut encore faire des essaies dessus ou non
bool sudoku_valide[NB_SUDOKU] = {true,true,true,true};

//les String des réponses pour les 4 sudokus
char reponse1[100];
char reponse2[100];
char reponse3[100];
char reponse4[100];

//Pour savoir s'il y a un doublon sur une ligne / une colonne / une sous-grille
//tableau de NB_SUDOKU pour les 4 sudokus
DoublonResultat rep_doublon_ligne[NB_SUDOKU];
DoublonResultat rep_doublon_colonne[NB_SUDOKU];
DoublonResultat rep_doublon_bloc[NB_SUDOKU];

//Tableau contenant les 4 sudokus
int sudoku[NB_SUDOKU][TAILLE_LECTURE][TAILLE_LECTURE];



/*-----------------------------------------------*/
/*           Début de la fonction Main           */

int main(int argc, char *argv[])
{
	int i = 0; //Cursor utilisé dans des boucles
  	int j = 0; //Cursor utilisé dans des boucles
  	int ligne = 0; //Va compter la ligne où l'on se trouve dans le sudoku
  	int colonne = 0; //Va compter la colonne où l'on se trouve dans le sudoku
  	int nb_sudoku = 0; //Va compter le nombre de sudoku

  	char chaine[100] = ""; // Chaîne vide permettant de lire une ligne complète du sudoku

	pthread_t tid_ligne [NB_COLONNES_TOTAL]; //Tableau regroupant les threads gérant les lignes
	pthread_t tid_colonne [NB_COLONNES_TOTAL]; //Tableau regroupant les threads gérant les colonnes
	pthread_t tid_bloc [NB_COLONNES_TOTAL]; //Tableau regroupant les threads gérant les blocs
    pthread_t tid_taille9x9 [NB_SUDOKU]; //Tableau regroupant les threads gérant la taille des sudokus
    pthread_t tid_caractere [NB_SUDOKU]; //Tableau regroupant les threads gérant la validités des caractères
	pthread_attr_t attr; // set of attributes for the thread

  	//On unitialise les réponses qui resteront inchangées si tout va bien
	sprintf(reponse1,"Bravo! Votre Sudoku est valide!");
	sprintf(reponse2,"Bravo! Votre Sudoku est valide!");
	sprintf(reponse3,"Bravo! Votre Sudoku est valide!");
	sprintf(reponse4,"Bravo! Votre Sudoku est valide!");

	//On unitilise les doublons trouvés comme 'false' -> donc non trouvé
	for ( i = 0; i < NB_SUDOKU ; i++ )
	{
		rep_doublon_colonne[i].trouver = false;
	}
	for ( i = 0; i < NB_SUDOKU ; i++ )
	{
		rep_doublon_ligne[i].trouver = false;
	}
	for ( i = 0; i < NB_SUDOKU ; i++ )
	{
		rep_doublon_bloc[i].trouver = false;
	}
	

	//On initialise les tableaux
	//Important pour gérer la taille 9x9
	for ( i = 0 ; i < NB_SUDOKU ; ++i )
	 {
	 	for ( ligne = 0 ; ligne < TAILLE_LECTURE ; ++ligne )
	 	{
	 		for ( colonne = 0 ; colonne < TAILLE_LECTURE ; ++colonne )
	 		{
	 			sudoku[i][ligne][colonne] = FIN_LECTURE;
	 		}
	 	}
	 }
	 colonne = 0;
	 ligne = 0 ;

  	


	//On teste si on a un fichier .txt passé en argument
	if (argc != 2) {
		printf("usage: a.out <file.txt>\n"); //Sinon on donne la synthaxe à l'utilisateur
		return -1;
	}

	//On demande à ouvrir le fichier .txt en lecture seule
	FILE* fichier = NULL;
    fichier = fopen(argv[1], "r");

    //Erreur si on n'arrive pas à lire le fichier (exemple: fichier inexistant)
    if (fichier == NULL)
    {
        printf("Erreur lors de la lecture du fichier .txt\n");
		return -1;
    }

 	  
    while (fgets(chaine, 100, fichier) != NULL ) // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
    {
    	//On traite ici la ligne que l'on vient de lire
    	for ( i = 0 ; i < TAILLE_LECTURE*2 && chaine[i] != '\n' ; i++ )
    	{
    		//i%2 == 0 pour sauter les espaces entre les caractères
    		if ( i%2 == 0 && colonne < TAILLE_LECTURE && ligne < TAILLE_LECTURE)
    		{
    			//Nombre valide
    			if ( chaine[i] >= '1' && chaine[i] <= '9' )
    				sudoku[nb_sudoku][ligne][colonne] = chaine[i] - '0'; //On convertie un char en int
    			//Caractère spéciale ou fin de ligne
    			else if ( chaine[i] == ' ' )
    				sudoku[nb_sudoku][ligne][colonne] = ESPACE;
    			//Caractère non entier ou 0
    			else if ( chaine[i] == '0' || ( chaine[i] >= 'a' && chaine[i] <= 'z' ) || ( chaine[i] >= 'A' && chaine[i] <= 'Z' ) )
    				sudoku[nb_sudoku][ligne][colonne] = CARACTERE;
    			//Sinon -> forcément un caractère spécial -> non admis
    			else if ( chaine[i] != '\r')
    			{
    				sudoku[nb_sudoku][ligne][colonne] = SPECIAL;
    			}
    			colonne++;    		
    		}
    	}

    	//Si la ligne se fini trop tôt on la complète avec des espaces
    	while (colonne < TAILLE_COLONNE && ligne < TAILLE_COLONNE )
		{
			sudoku[nb_sudoku][ligne][colonne] = ESPACE;
			colonne++;

		}
	    colonne = 0;
	    ligne++;

	    //Si on passe au sudoku suivant
    	if ( i == 1 )
		{
			//S'il manquait des lignes au sudoku précédent on le complète avec des espaces
			while (ligne < TAILLE_COLONNE)
			{
				while ( colonne < TAILLE_COLONNE )
				{
					sudoku[nb_sudoku][ligne][colonne] = ESPACE;
					colonne++;
				}
				ligne++;
			}
			ligne = 0;
			nb_sudoku++; //On passe au sudoku suivant
		}
 	}

    //On pense à fermer le fichier en lecteur (permet de libérer l'espace mémoire pointé par 'fichier')
    fclose(fichier);


     /* ---------------------------------------------------- */
    /*    ON COMMENCE A TRAITER LE SUDOKU A PARTIR D'ICI    */

    pthread_attr_init(&attr);

    //Ces deux lignes permettent de passer en paramètre dans les threads le numéro du sudoku à traiter
    int num_sudoku[NB_SUDOKU] = {0,1,2,3};
    int * data_nombre_sudoku = num_sudoku;

    //On crée tous les paramètres à passer pour traiter TOUTES les lignes de TOUS les sudokus
    Parameters *data_ligne = (Parameters *) malloc (sizeof (Parameters) * NB_COLONNES_TOTAL );
    //Pour chaque sudoku
    for ( i = 0 ; i < NB_SUDOKU ; i++ )
    {
    	//Et pour chaque ligne du sudoku en question
    	for ( j = 0 ; j < TAILLE_COLONNE ; j++ )
    	{
    		(data_ligne + TAILLE_COLONNE*i + j)->ligne = j; //La ligne à traiter
    		(data_ligne + TAILLE_COLONNE*i + j)->colonne = 0;
    		(data_ligne + TAILLE_COLONNE*i + j)->numSudoku = i; //indique le numéro du sudoku à traiter
    	}
    }

    //On crée tous les paramètres à passer pour traiter TOUTES les colonnes de TOUS les sudokus
    Parameters *data_colonne = (Parameters *) malloc (sizeof (Parameters) * NB_COLONNES_TOTAL );
    for ( i = 0 ; i < NB_SUDOKU ; i++ ) //Pour chaque sudoku
    {
    	for ( j = 0 ; j < TAILLE_COLONNE ; j++ ) //Et pour chaque ligne du sudoku en question
    	{
    		(data_colonne + TAILLE_COLONNE*i + j)->ligne = 0;
    		(data_colonne + TAILLE_COLONNE*i + j)->colonne = j; //La colonne à traiter
    		(data_colonne + TAILLE_COLONNE*i + j)->numSudoku = i; //indique le numéro du sudoku à traiter
    	}
    }

    //On crée tous les paramètres à passer pour traiter TOUTES les sous-grilles de TOUS les sudokus
    Parameters *data_bloc = (Parameters *) malloc (sizeof (Parameters) * NB_COLONNES_TOTAL );
    for ( i = 0 ; i < NB_SUDOKU ; i++ ) //Pour chaque sudoku
    {
    	ligne = 0;
		colonne = 0;
		for ( j = 0 ; j < TAILLE_COLONNE ; j++ ) //Et pour chaque sous-grille du sudoku en question
		{
			(data_bloc + TAILLE_COLONNE*i + j )->ligne = TAILLE_BLOC * ligne ;
			(data_bloc + TAILLE_COLONNE*i + j )->colonne = TAILLE_BLOC * colonne ;	
			/*Coordonnées obtenues :
			(0,0)|(0,3)|(0,6)
			(3,0)|(3,3)|(3,6)
			(6,0)|(6,3)|(6,6)
			Soit les coordonnées des 1er points de chaques blocs*/

			(data_bloc + TAILLE_COLONNE*i + j )->numSudoku = i; //indique le numéro du sudoku à traiter
			colonne++;
			if (colonne >= TAILLE_BLOC ) //Quand on arrive à colonne == 3 (TAILLE_BLOC) on repart à zéro sur la ligne suivante
			{
				ligne++;
				colonne = 0;
			}
		}
    }

    //On regarde si tous les sudokus sont bien de taille 9x9
	for ( i = 0 ; i < NB_SUDOKU ; i++ )
	{
		pthread_create(&tid_taille9x9[i],&attr,runner_taille9x9,(data_nombre_sudoku + i));
	}

    //On attend que tous les threads soient fini avant de poursuivre
    for ( i = 0; i < NB_SUDOKU ; i++)
   	{
	    pthread_join(tid_taille9x9[i],NULL);
	}

	//On regarde si tous les sudokus possèdent bien uniquement des entiers valides
	for ( i = 0 ; i < NB_SUDOKU ; i++ )
	{
		pthread_create(&tid_caractere[i],&attr,runner_caractere,(data_nombre_sudoku + i));
	}

    //On attend que tous les threads soient fini avant de poursuivre
    for ( i = 0; i < NB_SUDOKU; i++)
   	{
	   pthread_join(tid_caractere[i],NULL);
	}
	


    //On lance tous les threads gérant les lignes + les colonnes + les sous-grilles
	for ( i = 0 ; i < NB_COLONNES_TOTAL ; i++ )
	{
		pthread_create(&tid_ligne[i],&attr,runner_ligne,(data_ligne + i));
		pthread_create(&tid_colonne[i],&attr,runner_colonne,(data_colonne + i));
		pthread_create(&tid_bloc[i],&attr,runner_bloc,(data_bloc + i));
	}
	
    

    //On attend que tous les threads soient fini avant de poursuivre
    for ( i = 0 ; i < NB_COLONNES_TOTAL; i++)
   	{
	    pthread_join(tid_ligne[i],NULL); //Tous les threads traitant des lignes
	    pthread_join(tid_colonne[i],NULL); //Tous les threads traitant des colonnes
	    pthread_join(tid_bloc[i],NULL); //Tous les threads traitant des blocs
	}

	for ( i = 0 ; i < NB_SUDOKU ; i++ ) //Pour chaque sudoku
	{
		if (rep_doublon_ligne[i].trouver && sudoku_valide[i]) //S'il est toujours valide et qu'on a trouvé un doublon dans une ligne
		{
			//On ré-écrit sa réponse
			if ( i == 0 ) //Si c'est le 1er sudoku etc.
				sprintf(reponse1,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_ligne[i].valeur,rep_doublon_ligne[i].colonne1,rep_doublon_ligne[i].ligne1,rep_doublon_ligne[i].colonne2,rep_doublon_ligne[i].ligne2);
			else if ( i == 1 )
				sprintf(reponse2,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_ligne[i].valeur,rep_doublon_ligne[i].colonne1,rep_doublon_ligne[i].ligne1,rep_doublon_ligne[i].colonne2,rep_doublon_ligne[i].ligne2);
			else if ( i == 2 )
				sprintf(reponse3,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_ligne[i].valeur,rep_doublon_ligne[i].colonne1,rep_doublon_ligne[i].ligne1,rep_doublon_ligne[i].colonne2,rep_doublon_ligne[i].ligne2);
			else
				sprintf(reponse4,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_ligne[i].valeur,rep_doublon_ligne[i].colonne1,rep_doublon_ligne[i].ligne1,rep_doublon_ligne[i].colonne2,rep_doublon_ligne[i].ligne2);
		}
	}

	for ( i = 0 ; i < NB_SUDOKU ; i++ ) //Pour chaque sudoku
	{
		if (rep_doublon_colonne[i].trouver && sudoku_valide[i]) //S'il est toujours valide et qu'on a trouvé un doublon dans une colonne
		{
			//On ré-écrit sa réponse
			if ( i == 0 ) //Si c'est le 1er sudoku etc.
				sprintf(reponse1,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_colonne[i].valeur,rep_doublon_colonne[i].colonne1,rep_doublon_colonne[i].ligne1,rep_doublon_colonne[i].colonne2,rep_doublon_colonne[i].ligne2);
			else if ( i == 1 )
				sprintf(reponse2,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_colonne[i].valeur,rep_doublon_colonne[i].colonne1,rep_doublon_colonne[i].ligne1,rep_doublon_colonne[i].colonne2,rep_doublon_colonne[i].ligne2);
			else if ( i == 2 )
				sprintf(reponse3,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_colonne[i].valeur,rep_doublon_colonne[i].colonne1,rep_doublon_colonne[i].ligne1,rep_doublon_colonne[i].colonne2,rep_doublon_colonne[i].ligne2);
			else
				sprintf(reponse4,"Il y a un doublon dans la grille 9x9\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_colonne[i].valeur,rep_doublon_colonne[i].colonne1,rep_doublon_colonne[i].ligne1,rep_doublon_colonne[i].colonne2,rep_doublon_colonne[i].ligne2);
		}
	}
	
	for ( i = 0 ; i < NB_SUDOKU ; i++ ) //Pour chaque sudoku
	{
		if (rep_doublon_bloc[i].trouver && sudoku_valide[i]) //S'il est toujours valide et qu'on a trouvé un doublon dans une sous-grille
		{			
			//On ré-écrit sa réponse
			if ( i == 0 ) //Si c'est le 1er sudoku etc.
				sprintf(reponse1,"Il y a un doublon dans une sous-grille 3x3\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_bloc[i].valeur,rep_doublon_bloc[i].colonne1,rep_doublon_bloc[i].ligne1,rep_doublon_bloc[i].colonne2,rep_doublon_bloc[i].ligne2);
			else if ( i == 1 )
				sprintf(reponse2,"Il y a un doublon dans une sous-grille 3x3\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_bloc[i].valeur,rep_doublon_bloc[i].colonne1,rep_doublon_bloc[i].ligne1,rep_doublon_bloc[i].colonne2,rep_doublon_bloc[i].ligne2);
			else if ( i == 2 )
				sprintf(reponse3,"Il y a un doublon dans une sous-grille 3x3\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_bloc[i].valeur,rep_doublon_bloc[i].colonne1,rep_doublon_bloc[i].ligne1,rep_doublon_bloc[i].colonne2,rep_doublon_bloc[i].ligne2);
			else
				sprintf(reponse4,"Il y a un doublon dans une sous-grille 3x3\n->le chiffre '%d' apparaît aux positions (%d,%d) et (%d,%d)",rep_doublon_bloc[i].valeur,rep_doublon_bloc[i].colonne1,rep_doublon_bloc[i].ligne1,rep_doublon_bloc[i].colonne2,rep_doublon_bloc[i].ligne2);
		}
	}




   	//On affiche les résultats obtenues
    printf("Sudoku 1 :\n");
    printf("%s\n\n", reponse1 ); //Pour le sudoku 1
    printf("Sudoku 2 :\n");
    printf("%s\n\n", reponse2 ); //Pour le sudoku 2
    printf("Sudoku 3 :\n");
    printf("%s\n\n", reponse3 ); //Pour le sudoku 3
    printf("Sudoku 4 :\n");
    printf("%s\n", reponse4 ); //Pour le sudoku 4

    //Avant de finir on pense à libérer la mémoire
    data_nombre_sudoku = NULL;
    free(data_ligne);
    free(data_colonne);
    free(data_bloc);

	return 0;
}

/*--------------------------------------------------------------*/
/*           Début des fonctions traitant des threads           */

//threads vérifiant la taille des sudokus
void *runner_taille9x9(void *param) 
{
	//On récupère les valeurs passées en paramètre
	int* par = (int*) param;
	int num_sudoku = *par ;

	int ligne;
	int colonne;
	int tmp;

	//Pour chaque ligne...
	for ( ligne = 0 ; ligne < TAILLE_COLONNE && sudoku_valide[num_sudoku] ; ligne++ )
	{
		tmp = 0;
		// Et pour chaque colonne...
		for ( colonne = 0 ; colonne < TAILLE_COLONNE && sudoku_valide[num_sudoku] ; colonne++ )
		{
			// On regarde si on trouve un espace dans le sudoku...
			if ( sudoku[num_sudoku][ligne][colonne] == ESPACE )
			{
				tmp = colonne;
				//Puis on regarde s'il n'y a que des espaces qui suivent
				while ( tmp < 9 && sudoku[num_sudoku][ligne][tmp] == ESPACE)
					tmp++;
			}
			//Si oui alors on change la réponse
				//Ou si il reste des caractères en position FIN_LECTURE
			if ( tmp == 9 || sudoku[num_sudoku][ligne][TAILLE_COLONNE] != FIN_LECTURE || sudoku[num_sudoku][TAILLE_COLONNE][0] != FIN_LECTURE )
			{
				sudoku_valide[num_sudoku] = false; //Et on s'arrête là
				if ( num_sudoku == 0 )
					sprintf(reponse1,"La taille de la grille de Sudoku devrait être 9x9");
				else if ( num_sudoku == 1 )
					sprintf(reponse2,"La taille de la grille de Sudoku devrait être 9x9");
				else if ( num_sudoku == 2 )
					sprintf(reponse3,"La taille de la grille de Sudoku devrait être 9x9");
				else
					sprintf(reponse4,"La taille de la grille de Sudoku devrait être 9x9");
			}
		}
	}

	pthread_exit(0);
}

//threads vérifiant ce qu'il a été écrit dans le sudoku
void *runner_caractere(void *param) 
{
	//On récupère les valeurs passées en paramètre
	int* par = (int*) param;
	int num_sudoku = *par ;

	int ligne;
	int colonne;

	//Pour chaque ligne...
	for ( ligne = 0 ; ligne < TAILLE_COLONNE && sudoku_valide[num_sudoku] ; ligne++ )
	{
		// Et pour chaque colonne...
		for ( colonne = 0 ; colonne < TAILLE_COLONNE && sudoku_valide[num_sudoku]  ; colonne++ )
		{
			// On regarde si on trouve un caractère dans le sudoku
			if ( sudoku[num_sudoku][ligne][colonne] == CARACTERE )
			{
				//ERREUR CARACTERE

				//Si oui alors on change la réponse et on s'arrête là
				sudoku_valide[num_sudoku] = false;
				if ( num_sudoku == 0 )
					sprintf(reponse1, "la case (%d,%d) contient un caractère non-entier ", (colonne + 1) , (ligne + 1) );
				else if ( num_sudoku == 1 )
					sprintf(reponse2, "la case (%d,%d) contient un caractère non-entier ", (colonne + 1) , (ligne + 1) );
				else if ( num_sudoku == 2 )
					sprintf(reponse3, "la case (%d,%d) contient un caractère non-entier ", (colonne + 1) , (ligne + 1) );
				else
					sprintf(reponse4, "la case (%d,%d) contient un caractère non-entier ", (colonne + 1) , (ligne + 1) );
			}
			// Ou un caractère spécial dans le sudoku...
			else if ( sudoku[num_sudoku][ligne][colonne] == SPECIAL || sudoku[num_sudoku][ligne][colonne] == ESPACE )
			{
				//ERREUR SPECIAL

				//Si oui alors on change la réponse et on s'arrête là
				sudoku_valide[num_sudoku] = false;
				if ( num_sudoku == 0 )
					sprintf(reponse1, "la case (%d,%d) contient un caractère spécial non admis", (colonne + 1) , (ligne + 1) );
				else if ( num_sudoku == 1 )
					sprintf(reponse2, "la case (%d,%d) contient un caractère spécial non admis", (colonne + 1) , (ligne + 1) );
				else if ( num_sudoku == 2 )
					sprintf(reponse3, "la case (%d,%d) contient un caractère spécial non admis", (colonne + 1) , (ligne + 1) );
				else
					sprintf(reponse4, "la case (%d,%d) contient un caractère spécial non admis", (colonne + 1) , (ligne + 1) );
			}
		}
	}

	pthread_exit(0);
}

//threads vérifiant les lignes des sudokus
void *runner_ligne(void *param) 
{

	//On récupère les valeurs passées en paramètre
	Parameters* pos = (Parameters*) param;

	//On les ré-écrits pour faciliter la lecture et pour ne pas les modifiers
	int ligne = pos->ligne;
	int colonne = pos->colonne;
	int num_sudoku = pos->numSudoku ;

	//Tableau qui va permettre de déterminer les doublons
	int tab_filtre [TAILLE_COLONNE] = {0,0,0,0,0,0,0,0,0};

	//Nombre facilitant la lisibilité du code
	int nombre;
	int j;

	//doublon 1 va permettre de se souvenir de quelle case fut le 1er doulon
	//On écrira le 2e directement
	Doublon doublon1;
	doublon1.v = 0; //On inialise sa valeur

	//On compte la récurence de chaque chiffre
	for ( ; colonne < TAILLE_COLONNE && sudoku_valide[num_sudoku] ; colonne++ )
	{
		nombre = sudoku[num_sudoku][ligne][colonne];
		tab_filtre[nombre-1]++; //Récurence = +1
	}

	//On vérifie la récurence de chaque nombre
	for ( j = 0 ; j < TAILLE_COLONNE && !rep_doublon_ligne[num_sudoku].trouver && sudoku_valide[num_sudoku] ; j++ )
	{
		//S'il y en a une...
		if ( tab_filtre[j] >= 2 && !rep_doublon_ligne[num_sudoku].trouver )
		{
			//On va enregistrer les deux 1er doublons
			for ( colonne = pos->colonne ; colonne < TAILLE_COLONNE && !rep_doublon_ligne[num_sudoku].trouver && sudoku_valide[num_sudoku] ; colonne++ )
			{
				//La valeur du doublons en question
				if ( sudoku[num_sudoku][ligne][colonne] == (j+1) && !rep_doublon_ligne[num_sudoku].trouver )
				{
					//Si doublon1.v est toujours à l'état initialisé on l'enregistre en tant que 1er doublon
					if ( doublon1.v == 0 )
					{
						doublon1.v = sudoku[num_sudoku][ligne][colonne]; // Sa valeur
						doublon1.l = ligne + 1;	//Sa ligne
						doublon1.c = colonne + 1; //Sa colonne
					}
					else
					{
						rep_doublon_ligne[num_sudoku].trouver = true; //On le déclare comme trouvé -> On s'arrête là
						rep_doublon_ligne[num_sudoku].valeur = doublon1.v; //La valeur du doublon
						rep_doublon_ligne[num_sudoku].ligne1 = doublon1.l; //La position 1
						rep_doublon_ligne[num_sudoku].colonne1 = doublon1.c;
						rep_doublon_ligne[num_sudoku].ligne2 = ligne + 1; //Et la position 2
						rep_doublon_ligne[num_sudoku].colonne2 = colonne + 1;
					}
				}
			}
		}
	}
	pthread_exit(0);
}

//threads vérifiant les lignes des sudokus
void *runner_colonne(void *param)
{
	//On récupère les valeurs passées en paramètre
	Parameters* pos = (Parameters*) param;

	//On les ré-écrits pour faciliter la lecture et pour ne pas les modifiers
	int ligne = pos->ligne;
	int colonne = pos->colonne;
	int num_sudoku = pos->numSudoku ;

	//Tableau qui va permettre de déterminer les doublons
	int tab_filtre [TAILLE_COLONNE] = {0,0,0,0,0,0,0,0,0};

	//Nombre facilitant la lisibilité du code
	int nombre;
	int j;

	Doublon doublon1;
	doublon1.v = 0;


	//On compte la récurence de chaque chiffre
	for ( ; ligne < TAILLE_COLONNE && sudoku_valide[num_sudoku]; ligne++ )
	{
		nombre = sudoku[num_sudoku][ligne][colonne];
		tab_filtre[nombre-1]++; //Récurence = +1
	}

	//IDEM/PARALLELE A LA PARTIE "runner_ligne"
	
	//On vérifie la récurence de chaque nombre
	for ( j = 0 ; j < TAILLE_COLONNE && !rep_doublon_colonne[num_sudoku].trouver && sudoku_valide[num_sudoku]; j++ )
	{
		//S'il y en a une...
		if ( tab_filtre[j] >= 2 && !rep_doublon_colonne[num_sudoku].trouver)
		{
			//On va enregistrer les deux 1er doublons
			for ( ligne = pos->ligne ; ligne < TAILLE_COLONNE && !rep_doublon_colonne[num_sudoku].trouver && sudoku_valide[num_sudoku]; ligne++ )
			{
				//La valeur du doublons en question
				if ( sudoku[num_sudoku][ligne][colonne] == (j+1) && !rep_doublon_colonne[num_sudoku].trouver )
				{
					//Si doublon1.v est toujours à l'état initialisé on l'enregistre en tant que 1er doublon
					if ( doublon1.v == 0 )
					{
						doublon1.v = sudoku[num_sudoku][ligne][colonne];
						doublon1.l = ligne + 1;
						doublon1.c = colonne + 1;
					}
					else
					{
						rep_doublon_colonne[num_sudoku].trouver = true; //On le déclare comme trouvé -> On s'arrête là
						rep_doublon_colonne[num_sudoku].valeur = doublon1.v;
						rep_doublon_colonne[num_sudoku].ligne1 = doublon1.l;
						rep_doublon_colonne[num_sudoku].colonne1 = doublon1.c;
						rep_doublon_colonne[num_sudoku].ligne2 = ligne + 1;
						rep_doublon_colonne[num_sudoku].colonne2 = colonne + 1;
					}
				}
			}
		}
	}
	pthread_exit(0);
}

//threads vérifiant les lignes des sudokus
void *runner_bloc(void *param)
{

	//On récupère les valeurs passées en paramètre
	Parameters* pos = (Parameters*) param;

	//On les ré-écrits pour faciliter la lecture et pour ne pas les modifiers
	int ligne = pos->ligne;
	int colonne = pos->colonne;
	int num_sudoku = pos->numSudoku ;

	//Tableau qui va permettre de déterminer les doublons
	int tab_filtre [TAILLE_COLONNE] = {0,0,0,0,0,0,0,0,0};

	//Nombre facilitant la lisibilité du code
	int j;
	int nombre;

	Doublon doublon1;
	doublon1.v = 0;

	//On vérifie la récurence de chaque nombre
	// Tableau à 2-dimensions => 3x3
	//Pour i allant de 'pos->ligne'  à 'pos->ligne + 3' (taille du bloc)
	for ( ; ligne < ( pos->ligne + TAILLE_BLOC ) && sudoku_valide[num_sudoku] ; ligne++ )
	{
		//Pour j allant de 'pos->colonne'  à 'pos->colonne + 3' (taille du bloc)
		for ( colonne = pos->colonne ; colonne < ( pos->colonne + TAILLE_BLOC ) && sudoku_valide[num_sudoku] ; colonne++ )
		{
			nombre = sudoku[num_sudoku][ligne][colonne];
			tab_filtre[nombre-1]++;  //Récurence = +1
		}
	}

	//On vérifie la récurence de chaque nombre
	for ( j = 0 ; j < TAILLE_COLONNE && !rep_doublon_bloc[num_sudoku].trouver && sudoku_valide[num_sudoku] ; j++ )
	{
		if ( tab_filtre[j] >= 2 && !rep_doublon_bloc[num_sudoku].trouver)
		{
			for ( ligne = pos->ligne ; ligne < ( pos->ligne + TAILLE_BLOC ) && !rep_doublon_bloc[num_sudoku].trouver && sudoku_valide[num_sudoku] ; ligne++ )
			{
				//Pour j allant de 'pos->colonne'  à 'pos->colonne + 3' (taille du bloc)
				for ( colonne = pos->colonne ; colonne < ( pos->colonne + TAILLE_BLOC )  && !rep_doublon_bloc[num_sudoku].trouver && sudoku_valide[num_sudoku] ; colonne++ )
				{
					//S'il y en a une...
					if ( sudoku[num_sudoku][ligne][colonne] == (j+1) && !rep_doublon_bloc[num_sudoku].trouver )
					{
						//Si doublon1.v est toujours à l'état initialisé on l'enregistre en tant que 1er doublon
						if ( doublon1.v == 0 )
						{
							doublon1.v = sudoku[num_sudoku][ligne][colonne];
							doublon1.l = ligne + 1;
							doublon1.c = colonne + 1;
						}
						else
						{
							rep_doublon_bloc[num_sudoku].trouver = true; //On le déclare comme trouvé -> On s'arrête là
							rep_doublon_bloc[num_sudoku].valeur = doublon1.v;
							rep_doublon_bloc[num_sudoku].ligne1 = doublon1.l;
							rep_doublon_bloc[num_sudoku].colonne1 = doublon1.c;
							rep_doublon_bloc[num_sudoku].ligne2 = ligne + 1;
							rep_doublon_bloc[num_sudoku].colonne2 = colonne + 1;
						}
					}
				}
			}
		}
	}

	pthread_exit(0);
}