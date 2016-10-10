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
#include <math.h>
#include "osada.h"



int tamanioFileSystem;

void crearArchivo(char* rutaFileSystem,char* nombreArchivoNuevo,int tamanio,int directorioPadre);
int buscarPrimerBloqueVacio();
int calcularTamanioDeArchivo(FILE* archivoAMapear);
void* mapearArchivoMemoria(FILE* archivo);
void inicializarBloqueCentral();


typedef struct{
	osada_header* header;
	t_bitarray* bitmap;
	osada_file tablaDeArchivos[1024];
	int* tablaDeAsignaciones;
	void* bloquesDeDatos;
}osada_bloqueCentral;




typedef struct{
	osada_file_state state;
	char* nombreDeArchivo;


};

osada_bloqueCentral* disco;
int tamanioDisco;
char* rutaDisco;

#endif /* POKEDEXSERVER_H_ */



