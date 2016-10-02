/*
 * pokedexServer.h
 *
 */

#ifndef POKEDEXSERVER_H_
#define POKEDEXSERVER_H_

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include "conexiones.h"
#include <commons/bitarray.h>
#include <time.h>
#include <sys/mman.h>



int tamanioFileSystem;

void crearArchivo(char* rutaFileSystem,char* nombreArchivoNuevo,int tamanio,int directorioPadre);
int buscarPrimerBloqueVacio();
int calcularTamanioDeArchivo(FILE* archivoAMapear);
void* mapearArchivoMemoria(FILE* archivo);


#endif /* POKEDEXSERVER_H_ */



