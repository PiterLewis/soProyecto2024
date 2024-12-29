#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);
		 } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
	     if (strcmp(orden,"dir")==0) {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
            }
         else if (strcmp(orden,"bytemaps")==0) {
            Printbytemaps(&ext_bytemaps);
            continue;
            }
         else if (strcmp(orden,"rename")==0) {
            Renombrar(&directorio,&ext_blq_inodos,argumento1,argumento2);
            Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
            continue;
            }
         else if (strcmp(orden,"info")==0) {
            LeeSuperBloque(&ext_superblock);
            continue;
            }
         else if (strcmp(orden,"salir")==0) {
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
            }
         else if (strcmp(orden,"imprimir")==0) {
            Imprimir(&directorio,&ext_blq_inodos,&memdatos,argumento1);
            continue;
            }
         else if (strcmp(orden,"remove")==0) {
            Borrar(&directorio,&ext_blq_inodos,&ext_bytemaps,&ext_superblock,argumento1,fent);
            continue;
            }
         else if (strcmp(orden,"copy")==0) {
            Copiar(&directorio,&ext_blq_inodos,&ext_bytemaps,&ext_superblock,&memdatos,argumento1,argumento2,fent);
            continue;
            }
         printf ("Comando no reconocido\n");

         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los 
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
         }

     }
}

// Funcion para imprimir los bytemaps
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
     int i,j;
     printf ("Bytemaps: \n");
     printf ("Bytemap inodos: \n");
     for (i=0;i<MAX_INODOS;i++){
         printf ("%3d.- %d\t",i,ext_bytemaps->bmap_inodos[i]);
         if (i%8 == 7)
            printf ("\n");
     }
     printf ("Bytemap bloques: \n");
     for (i=0;i<MAX_BLOQUES_PARTICION;i++){
         printf ("%3d.- %d\t",i,ext_bytemaps->bmap_bloques[i]);
         if (i%8 == 7)
            printf ("\n");
     }
}

// Funcion para comprobar el comando introducido
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){

   // para bytemaps, info, dir, salir solo se necesita un comando
   if (sscanf(strcomando,"%s",orden)==1)
      return 0;
   // para los comandos rename, remove, copy se necesitan dos comandos
   if (sscanf(strcomando,"%s %s",orden,argumento1)==2)
      return 0;
   // para el comando copy se necesitan tres comandos
   if (sscanf(strcomando,"%s %s %s",orden,argumento1,argumento2)==3)
      return 0;
   return -1;


}

// funcion para leer el superbloque
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){

   //informacion del superbloque como en el ejemplo 
     printf("Bloques: %d\n",psup->s_blocks_count);
     printf("inodos: %d\n",psup->s_inodes_count);
     printf("inodos libres: %d\n",psup->s_free_inodes_count);
     printf("Bloques libres: %d\n",psup->s_free_blocks_count);
     printf("Primer bloque de datos: %d\n",psup->s_first_data_block);
     printf("Tamaño del bloque: %d\n",psup->s_block_size);
}

// funcion para buscar un fichero

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
    int i;

    // recorrer el directorio para buscar el fichero
    for (i=0;i<MAX_FICHEROS;i++){
        if (strcmp(directorio[i].dir_nfich,nombre)==0){
           return directorio[i].dir_inodo;
        }
    }
    return -1;
}

// funcion para imprimir el directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
     int i;
     printf ("Información del directorio: \n");
     for (i=0;i<MAX_FICHEROS;i++){
         if (inodos->blq_inodos[i].size_fichero != 0){
            printf ("Nombre: %s\n",directorio[i].dir_nfich);
            printf ("Tamaño: %d\n",inodos->blq_inodos[i].size_fichero);
         }
     }
}

// funcion para renombrar un fichero
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
    int i;

    // recorrer el directorio para buscar el fichero
    for (i=0;i<MAX_FICHEROS;i++){
        if (strcmp(directorio[i].dir_nfich,nombreantiguo)==0){
           strcpy(directorio[i].dir_nfich,nombrenuevo);
           return 0;
        }
    }
    return -1;
}

// funcion para imprimir un fichero
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
 // Comprobamos primero si el fichero existe
   int ninodo;
   ninodo = BuscaFich(directorio,inodos,nombre);
   if (ninodo == -1){
      printf ("El fichero no existe\n");
      return -1;
   }
   //concatenamos los bloques de datos
   int i;
   for (i=0;i<MAX_NUMS_BLOQUE_INODO;i++){
       if (inodos->blq_inodos[ninodo].i_nbloque[i] != NULL_BLOQUE){
          printf ("%s",memdatos[inodos->blq_inodos[ninodo].i_nbloque[i]].dato);
       }
   }
   printf ("\n");
   return 0;

}

// funcion para borrar un fichero

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){

      //comprobamos si existe 
      int ninodo;
      ninodo = BuscaFich(directorio,inodos,nombre);
      if (ninodo == -1){
         printf ("El fichero no existe\n");
         return -1;
      }
      //liberamos los bloques de datos
      int i;
      for (i=0;i<MAX_NUMS_BLOQUE_INODO;i++){
          if (inodos->blq_inodos[ninodo].i_nbloque[i] != NULL_BLOQUE){
             ext_bytemaps->bmap_bloques[inodos->blq_inodos[ninodo].i_nbloque[i]] = 0;
          }
      }
      //liberamos el inodo
      ext_bytemaps->bmap_inodos[ninodo] = 0;
      inodos->blq_inodos[ninodo].size_fichero = 0;
      // no sobreescribimos la infomacion guardada en los bloques
      
      return 0;

}

// funcion para copiar un fichero
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){

      //comprobamos si existe el fichero origen
      int ninodoorigen;
      ninodoorigen = BuscaFich(directorio,inodos,nombreorigen);
      if (ninodoorigen == -1){
         printf ("El fichero origen no existe\n");
         return -1;
      }
      //comprobamos si existe el fichero destino
      int ninododestino;
      ninododestino = BuscaFich(directorio,inodos,nombredestino);
      if (ninododestino != -1){
         printf ("El fichero destino ya existe\n");
         return -1;
      }
      //buscamos un inodo libre
      int i;
      for (i=0;i<MAX_INODOS;i++){
          if (ext_bytemaps->bmap_inodos[i] == 0){
             break;
          }
      }
      if (i == MAX_INODOS){
         printf ("No hay inodos libres\n");
         return -1;
      }
      //buscamos bloques libres
      int j;
      for (j=0;j<MAX_BLOQUES_DATOS;j++){
          if (ext_bytemaps->bmap_bloques[j] == 0){
             break;
          }
      }
      if (j == MAX_BLOQUES_DATOS){
         printf ("No hay bloques libres\n");
         return -1;
      }
      //copiamos la informacion del inodo
      ext_bytemaps->bmap_inodos[i] = 1;
      inodos->blq_inodos[i].size_fichero = inodos->blq_inodos[ninodoorigen].size_fichero;
      for (i=0;i<MAX_NUMS_BLOQUE_INODO;i++){
          inodos->blq_inodos[i].i_nbloque[i] = inodos->blq_inodos[ninodoorigen].i_nbloque[i];
      }
      //copiamos la informacion de los bloques de datos
      for (i=0;i<MAX_NUMS_BLOQUE_INODO;i++){
          if (inodos->blq_inodos[ninodoorigen].i_nbloque[i] != NULL_BLOQUE){
             ext_bytemaps->bmap_bloques[j] = 1;
               memcpy(memdatos[j].dato,memdatos[inodos->blq_inodos[ninodoorigen].i_nbloque[i]].dato,SIZE_BLOQUE);
      
          }
          }

}

// funcion para grabar los inodos y el directorio
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
     int i;
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     memcpy(&datosfich[3],directorio,SIZE_BLOQUE);
     memcpy(&datosfich[2],inodos,SIZE_BLOQUE);
     fseek(fich,SIZE_BLOQUE,SEEK_SET);
     fwrite(&datosfich, SIZE_BLOQUE, 2, fich);
}

// funcion para grabar los bytemaps
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

     memcpy(&datosfich[1],ext_bytemaps,SIZE_BLOQUE);
     fseek(fich,SIZE_BLOQUE,SEEK_SET);
     fwrite(&datosfich, SIZE_BLOQUE, 1, fich);
}

// funcion para grabar el superbloque

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

     memcpy(&datosfich[0],ext_superblock,SIZE_BLOQUE);
     fseek(fich,0,SEEK_SET);
     fwrite(&datosfich, SIZE_BLOQUE, 1, fich);
}

// funcion para grabar los datos
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
     fseek(fich,4*SIZE_BLOQUE,SEEK_SET);
     fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}
