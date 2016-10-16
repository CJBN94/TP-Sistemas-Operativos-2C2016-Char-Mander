/*
 * pokedexServer.h
 *
 */

#ifndef POKEDEXSERVER_H_
#define POKEDEXSERVER_H_

#define ULTIMO_BLOQUE -2
#define BLOQUE_VACIO -1

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

typedef enum{
	LEER_ARCHIVO = 0,
	CREAR_ARCHIVO,
	ESCRIBIR_ARCHIVO,
	BORRAR_ARCHIVO,
	CREAR_DIRECTORIO,
	BORRAR_DIRECTORIO,
	RENOMBRAR_ARCHIVO
} enum_operacion;


int tamanioFileSystem;


//OPERACIONES PRINCIPALES//

void leerArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer);
void crearArchivo(char* rutaArchivoNuevo);
void escribirOModificarArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer);
void borrarArchivos(char* rutaDeArchivo);
void crearDirectorio(char* rutaDirectorioPadre);
void borrarDirectoriosVacios(char* rutaDelDirectorioABorrar);
void renombrarArchivo(char* rutaDeArchivo,char* nuevoNombre);

//OPERACIONES SECUNDARIAS//
void completarTablaDeAsignaciones(int* tablaDeAsignaciones,int cantidadDeBloquesArchivo,int primerBloque);
int calcularTamanioDeArchivo(FILE* archivoAMapear);
void* mapearArchivoMemoria(FILE* archivo);
int buscarBloqueVacioEnElBitmap();
void inicializarBloqueCentral();
int* buscarSecuenciaBloqueDeDatos(osada_file archivo);
double calcularBloquesAPedir(int bytesRestantes);
int calcularUltimoBloque(int* secuencia);
int calcularPosicionDeEscrituraUltimoBloque(int cantidadRestanteAEscribir);
int llenarEspacioLibreUltimoBloque(int* secuencia,char* loQueVoyAEscribir);
osada_file buscarArchivoPorRuta(char* rutaAbsolutaArchivo);
void copiarArchivoNuevoEnMemoria(void* fsMapeado,int* tablaDeAsignaciones,int primerBloque,int cantidadDeBloquesArchivo);
void seteoInicialTablaDeAsignaciones(int* tablaDeAsignaciones);

int revisarMismoNombre(osada_file archivoARenombrar, char* nuevoNombre);
int posicionArchivoPorRuta(char* rutaAbsolutaArchivo);
int contarCantidadDeDirectorios();
void borrarDirectoriosVacios();
char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo);
void escucharOperaciones(int operaciones);
typedef struct{
	osada_header* header;
	t_bitarray* bitmap;
	osada_file tablaDeArchivos[2048];
	int* tablaDeAsignaciones;
	osada_block* bloquesDeDatos;
}osada_bloqueCentral;




typedef struct{
	osada_file_state state;
	char* nombreDeArchivo;


}t_osadaState;

osada_bloqueCentral* disco;
int tamanioDisco;
char* rutaDisco;

#endif /* POKEDEXSERVER_H_ */



