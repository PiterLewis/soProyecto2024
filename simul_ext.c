#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "./cabeceras.h"
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

int main() {
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];
    
    int i, j;
    unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos = 0;
    FILE *fent;
    
    // Lectura del fichero completo de una sola vez
    fent = fopen("particion.bin", "r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
    
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);
    
    while (1) {
        printf(">> ");
        fgets(comando, LONGITUD_COMANDO, stdin);
        ComprobarComando(comando, orden, argumento1, argumento2);
        
        if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        } else if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
        } else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } else if (strcmp(orden, "rename") == 0) {
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
            grabardatos = 1;
        } else if (strcmp(orden, "imprimir") == 0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        } else if (strcmp(orden, "remove") == 0) {
            Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
            grabardatos = 1;
        } else if (strcmp(orden, "copy") == 0) {
            Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
            grabardatos = 1;
        } else if (strcmp(orden, "salir") == 0) {
            GrabarDatos(memdatos, fent);
            fclose(fent);
            return 0;
        } else {
            printf("Comando no reconocido\n");
        }
        
        // Escritura de metadatos en comandos rename, remove, copy
        if (grabardatos) {
            Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
            GrabarByteMaps(&ext_bytemaps, fent);
            GrabarSuperBloque(&ext_superblock, fent);
            GrabarDatos(memdatos, fent);
            grabardatos = 0;
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
     }
     printf("\n");
     printf("Bytemap bloques: \n");
     for (i=0;i<25;i++){
         printf ("%3d.- %d\t",i,ext_bytemaps->bmap_bloques[i]);
     }
}

// Funcion para comprobar el comando introducido
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    int num = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
    if (num == 1) {
        argumento1[0] = '\0';
        argumento2[0] = '\0';
    } else if (num == 2) {
        argumento2[0] = '\0';
    }
    return num;
}

// funcion para leer el superbloque
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){

   //informacion del superbloque como en el ejemplo 
     printf("bloques: %d\n",psup->s_blocks_count);
     printf("inodos: %d\n",psup->s_inodes_count);
     printf("inodos libres: %d\n",psup->s_free_inodes_count);
     printf("bloques libres: %d\n",psup->s_free_blocks_count);
     printf("primer bloque de datos: %d\n",psup->s_first_data_block);
     printf("tamaño del bloque: %d\n",psup->s_block_size);
}

// funcion para buscar un fichero

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    int i;
    for (i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

// funcion para imprimir el directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    int i, j;
    printf("Nombre\tTamaño\tInodo\tBloques\n");
    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            printf("%s\t%d\t%d\t", directorio[i].dir_nfich, inodos->blq_inodos[directorio[i].dir_inodo].size_fichero, directorio[i].dir_inodo);
            for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE) {
                    printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}

// funcion para renombrar un fichero
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    int index_old = BuscaFich(directorio, inodos, nombreantiguo);
    int index_new = BuscaFich(directorio, inodos, nombrenuevo);
    if (index_old == -1) {
        printf("ERROR: El fichero %s no existe\n", nombreantiguo);
        return -1;
    }
    if (index_new != -1) {
        printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
        return -1;
    }
    strcpy(directorio[index_old].dir_nfich, nombrenuevo);
    return 0;
}

// funcion para imprimir un fichero
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    // Busca el archivo en el directorio
    int index = BuscaFich(directorio, inodos, nombre);
    if (index == -1) {
        printf("ERROR: El fichero %s no existe\n", nombre);
        return -1;
    }

    // Obtén el inodo asociado al archivo
    unsigned short int ninodo = directorio[index].dir_inodo;
    printf("Contenido del fichero %s:\n", nombre);
    printf("Tamaño del archivo: %d bytes\n", inodos->blq_inodos[ninodo].size_fichero);

    // Depuración: Verifica los bloques asociados al inodo
    printf("Bloques asignados:\n");
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        unsigned short int bloque = inodos->blq_inodos[ninodo].i_nbloque[i];
        if (bloque != NULL_BLOQUE) {
            printf("Bloque %d: %d\n", i, bloque);
        } else {
            printf("Bloque %d: NULL_BLOQUE\n", i);
        }
    }

    // Concatenación final de los bloques para el contenido completo del archivo
    printf("\nContenido completo del fichero:\n");
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        unsigned short int bloque = inodos->blq_inodos[ninodo].i_nbloque[i];
        if (bloque != NULL_BLOQUE) {
            for (int j = 0; j < SIZE_BLOQUE; j++) {
                printf("%c", memdatos[bloque].dato[j]);
            }
        }
    }
    printf("\n");
    return 0;
}



// funcion para borrar un fichero

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich) {
    int index = BuscaFich(directorio, inodos, nombre);
    if (index == -1) {
        printf("ERROR: El fichero %s no existe\n", nombre);
        return -1;
    }

    unsigned short int ninodo = directorio[index].dir_inodo;

    
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        unsigned short int bloque = inodos->blq_inodos[ninodo].i_nbloque[i];
        if (bloque != NULL_BLOQUE) {
            ext_bytemaps->bmap_bloques[bloque] = 0;  // Liberar bloque en bytemap
            inodos->blq_inodos[ninodo].i_nbloque[i] = NULL_BLOQUE;  // Limpiar referencia a bloque en inodo
        }
    }

    // Liberar inodo
    inodos->blq_inodos[ninodo].size_fichero = 0;
    ext_bytemaps->bmap_inodos[ninodo] = 0;

    // Limpiar entrada en directorio
    directorio[index].dir_inodo = NULL_INODO;
    strcpy(directorio[index].dir_nfich, "");

    // Actualizar superbloque
    ext_superblock->s_free_blocks_count += MAX_NUMS_BLOQUE_INODO;
    ext_superblock->s_free_inodes_count++;

    printf("Fichero %s eliminado correctamente.\n", nombre);
    return 0;
}



// funcion para copiar un fichero
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    int index_origen = BuscaFich(directorio, inodos, nombreorigen);
    if (index_origen == -1) {
        printf("ERROR: El fichero %s no existe\n", nombreorigen);
        return -1;
    }

    if (BuscaFich(directorio, inodos, nombredestino) != -1) {
        printf("ERROR: El fichero %s ya existe\n", nombredestino);
        return -1;
    }

    // Buscar inodo libre
    unsigned short int nuevo_inodo = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            nuevo_inodo = i;
            break;
        }
    }
    if (nuevo_inodo == -1) {
        printf("ERROR: No hay inodos libres\n");
        return -1;
    }

    // Copiar bloques
    unsigned short int origen_inodo = directorio[index_origen].dir_inodo;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodos->blq_inodos[origen_inodo].i_nbloque[i] != NULL_BLOQUE) {
            for (int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    ext_bytemaps->bmap_bloques[j] = 1;
                    inodos->blq_inodos[nuevo_inodo].i_nbloque[i] = j;
                    memcpy(memdatos[j].dato, memdatos[inodos->blq_inodos[origen_inodo].i_nbloque[i]].dato, SIZE_BLOQUE);
                    break;
                }
            }
        }
    }

    // Actualizar inodo y directorio
    ext_bytemaps->bmap_inodos[nuevo_inodo] = 1;
    inodos->blq_inodos[nuevo_inodo].size_fichero = inodos->blq_inodos[origen_inodo].size_fichero;

    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            directorio[i].dir_inodo = nuevo_inodo;
            strcpy(directorio[i].dir_nfich, nombredestino);
            break;
        }
    }

    printf("Fichero %s copiado a %s\n", nombreorigen, nombredestino);
    return 0;
}


// funcion para grabar los inodos y el directorio
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE * 2, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

// funcion para grabar los bytemaps
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

// funcion para grabar el superbloque

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

// funcion para grabar los datos
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
     fseek(fich,4*SIZE_BLOQUE,SEEK_SET);
     fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}
