/*
 *
 * @author : Maxime Girard 
 * @CodePermanent : GIRM30058500
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.h"

#define MAX_SIZE_DATA 649999

/*
 * structure de donnée legere.
 */
 struct Element{
   char *file_path;
   int taille_element;
 };

ino_t tableau_inode[MAX_SIZE_DATA];
 
/*
 * Valide le nombre d'argument 
 * @param argc le nombre d'argument
 * @return 0 si argc < 2 
 * @return 1 si argc == 2
 */
int valider_nombre_arguments(int argc, const char path[])
{
  if(argc < 2)
  {
	print_erreur(ERR_FICHIER_REPERTOIRE_INEXISTANT);
  	return 0;
  }
  
  struct stat file_stat;
  if(stat(path, &file_stat) != 0)
  {
   print_erreur(ERR_FICHIER_REPERTOIRE_INEXISTANT);
   return 0;
  }
  return 1;
};


/*
 * Genere un fichier statistique du fichier specifie en argument.
 * @param const char path[] le chemin du fichier. 
 * @param struct stat * file la structure de donne pour les donnees statistiques 
 * @return 0 si une erreur s'est produite pendant l'ouverture.
 * @return 1 si l'ouverture s'est dérouler sans problème.
 */
int statify(const char path[], struct stat * file)
{ 
  struct stat file_stat;
  
  if(lstat(path, &file_stat) == 0 && S_ISLNK(file_stat.st_mode))
  {
    if(memcpy(file, &file_stat, sizeof(file_stat)) != NULL) return 2;
  }
  else if(stat(path, &file_stat) != 0) return 0;
  
  if(memcpy(file, &file_stat, sizeof(file_stat)) == NULL) return 0;
  return 1;
};


/*
 * Verifie que l'executable ai les permission nécéssaire (code octal 4, lecture user)
 * @param int st_mode l'entier qui représente le code binaire des permission du fichier / repertoire.
 * @return 0 si "r" est absent , pas les droits de lecture.
 * @return 1 si "r" est présent, les droits de lecture sont présent.
 */
int valider_permission_argument(const int st_mode)
{
 if (st_mode&S_IRUSR) return 1;
 else 
 {
  print_erreur(ERR_DROITS_INSUFFISANTS);
  return 0;
 }
};


/*
 * Détermine si un dossier est "." ou ".."
 * @param name le nom du dossier
 * @return 1 si oui , 0 sinon.
 */
int est_non_courant_ou_parent(char * name)
{
  if(((!strcmp(name, ".") == 0) && !strcmp(name, "..") == 0)) return 1;
  return 0;
};


/*
 * Retourne le nombre de fichiers contenus dans le repertoire passé en parametre de la fonction main.
 * @param nom_repertoire est le nom du repertoire passer en parametre a main().
 * @return le nombre d'élément dans nom_repertoire.
 */
int get_nombre_element_repertoire(char * nom_repertoire)
{
 int i = 0;
 DIR *dir;
 struct dirent *ent;

 if ((dir = opendir (nom_repertoire)) != NULL) 
 { 
  while ((ent = readdir (dir)) != NULL)
   if(est_non_courant_ou_parent(ent->d_name)) i++; 
  closedir (dir);
  return i;
 } 
 else /* n'a pas pu ouvrir */
  return -1;
 return 0; 
};


/*
 * ensemble_de_validation un wrapper pour la verification / validation des arguments 
 * @param argc le nombre d'argument
 * @param argv le tableau de char * (chaine de char) representant les arguments
 * @param nombre_element dans le dossier dont le path est argv[1]
 * @return 0 en cas d'erreur, 1 sinon.
 */
int ensemble_de_validation(int argc, char * argv[], int * nombre_element)
{
 struct stat file_stat;
 char file_path[FILENAME_MAX+1];
 
 if(!valider_nombre_arguments(argc, argv[1])) return 0; 
 
 strcpy(file_path, argv[1]);
 
 if(!statify(file_path, &file_stat)) return 0;
 if(!valider_permission_argument(file_stat.st_mode)) return 0;

 *nombre_element = get_nombre_element_repertoire(file_path);
 return 1;
};


/*
 * fonction verifie si l'inode à été examiné.
 * @param numero_inode le numero d'inode à verifier
 * @return 0 si l'inode a deja été examiner.
 * @return 1 l'inode n'a pas encore été examiné.
 */
int verification_double_inode(ino_t numero_inode)
{
 for(int i = 0 ; i < MAX_SIZE_DATA; i++)
   if(tableau_inode[i] == numero_inode) return 0;
 
 return 1;
}


/*
 * get_size_fichiers_dans_repertoire 
 * @param nom_repertoire le nom du repertoire principal passer en argument
 * @param nombre_element le nombre de fichiers/dossiers dans le repertoire de nom_repertoire
 * @param tableau_element[] tableau de struct Element 
 * @param indic entier indicateur utiliser par la récursion
 */
int get_size_fichiers_dans_repertoire(char * nom_repertoire, int nombre_element, struct Element tableau_element[], int indic)
{
  static int position = 0;
  static int position_alter = 0;

  int size_total_rep = 0, size_total = 0;
  struct dirent *ent;
  struct stat file_static;
  char file_path[FILENAME_MAX+1] = {0};
  
  strcpy(file_path, nom_repertoire);
  statify(file_path, &file_static);
  DIR *dir;
  
  if((dir = opendir (nom_repertoire)) != NULL) 
  { 
  while ((ent = readdir (dir)) != NULL)
  {
   if(est_non_courant_ou_parent(ent->d_name))
   {
    sprintf( file_path, "%s/%s", nom_repertoire, ent->d_name );
    int status = statify(file_path, &file_static); 

    if(S_ISREG(file_static.st_mode) || status == 2)
    {
      size_total += file_static.st_size;
      if(indic == 0 && verification_double_inode(file_static.st_ino))
      {
       tableau_inode[position_alter++] = file_static.st_ino;
       tableau_element[position].file_path = strcpy(malloc(sizeof(file_path)), file_path);
       tableau_element[position++].taille_element = file_static.st_size;
      }
    } 
    else if (S_ISDIR(file_static.st_mode))
    {
     size_total_rep = 4096;
     size_total_rep += get_size_fichiers_dans_repertoire(file_path, get_nombre_element_repertoire(file_path), tableau_element, 1);
     size_total += size_total_rep;
     
     if(indic == 0)
     { 
      tableau_element[position].file_path = strcpy(malloc(sizeof(file_path)), file_path);
      tableau_element[position++].taille_element = size_total_rep;
     }
    }
   } 
  }
   while ((closedir(dir) == -1) && (errno == EINTR));
  } 
  
  else return -1;
  return size_total;
};


/*
 * Tri à bulle , trie les entier de tab_a_trier en ordre décroissant.
 * @param tab_a_trier[] le tableau d'entier à trier
 * @param taille la taille du tableau à trier
 */
void tri_bulle(int tab_a_trier[], int taille)
{
  int i;
  int j;
  int tmp;

  for (i = 0; i < (taille - 1); ++i)
  {
   for (j = 0; j < taille - 1 - i; ++j )
   {
    if (tab_a_trier[j] < tab_a_trier[j+1])
    {
     tmp = tab_a_trier[j+1];
     tab_a_trier[j+1] = tab_a_trier[j];
     tab_a_trier[j] = tmp;
    }
   }
  }
};


/*
 * fonction qui retourne un tableau d'entier trier en ordre décroissant.
 * @param tab un tableau de structure Element 
 * @param size un entier représentant la taille de tab 
 */
void get_tab_trier(struct Element tab[], int size)
{
   int tableau_taille[size];
  
   for(int i = 0; i<size; i++)
    tableau_taille[i] = tab[i].taille_element;
     
   tri_bulle(tableau_taille,size);

   for(int i = 0; i < size; i++)
       for(int j = 0; j < size; j++)
       {
         if(tableau_taille[i] == tab[j].taille_element && tab[i].file_path != NULL)
         {
          print_element(tableau_taille[i], tab[j].file_path);
          break;
         }
       }
};


/*
 * fonction principale 
 * @param argc nombre d'argument à l'appel
 * @param argv[] le tableau de pointeur de char (un tableau de chaine de caractere).
 */
int main(int argc, char *argv[])
{
 int nombre_element, size_repertoire_courrant = 0; 
 
 if(!ensemble_de_validation(argc,argv, &nombre_element)) return EXIT_FAILURE;

 struct Element *tableau_element = malloc(sizeof(struct Element)*nombre_element);
 
 if((size_repertoire_courrant = get_size_fichiers_dans_repertoire(argv[1], nombre_element, tableau_element, 0)) == -1)
  return EXIT_FAILURE;

 get_tab_trier(tableau_element,nombre_element);
 free(tableau_element);

 return EXIT_SUCCESS;
};
