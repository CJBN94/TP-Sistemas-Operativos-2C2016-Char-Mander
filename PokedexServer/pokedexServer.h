/*
 * pokedexServer.h
 *
 */

#ifndef POKEDEXSERVER_H_
#define POKEDEXSERVER_H_

#define ULTIMO_BLOQUE -1
#define BLOQUE_VACIO -1
#define ROOT_DIRECTORY 65535


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
#include <commons/config.h>
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
	RENOMBRAR_ARCHIVO,
	LISTAR_ARCHIVOS,
	TRUNCAR_ARCHIVO,
	MOVER_ARCHIVO
} enum_operacion;


#include "conexiones.h"


int tamanioFileSystem;
char* RUTA_DISCO;
sem_t semaforos_permisos[2048];

//OPERACIONES PRINCIPALES//


void leerArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer);
/* Parametros
		   	   	   - Buffer(contenido real a leer),
		   	   	   - Ruta del Archivo,
		   	   	   - Offset (Punto de arranque),
		   	   	   - Tamaño del buffer.

		   	*/

void crearArchivo(char* rutaArchivoNuevo);
		/*Parametros:
		    		- Ruta del archivo (se separa el ultimo parametro para obtener el nombre del archivo)

		    */
void escribirOModificarArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer);
/*Parametros
		   		   	   	   - Buffer(contenido real a escribir),
		   		   	   	   - Ruta del Archivo,
		   		   	   	   - Offset (Punto de arranque),
		   		   	   	   - Tamaño del buffer.
		   */
void borrarArchivos(char* rutaDeArchivo);
/*Parametros
		   		   	   	   - Ruta del Archivo
		   */
void crearDirectorio(char* rutaDirectorioPadre);
/*Parametros
		   	   	   	   	   - Ruta del directorio a crear.
		   */
void borrarDirectoriosVacios();

void borrarDirectorioVacios(char* rutaDelDirectorioABorrar);
/*Parametros:
		     	 	 	 - Ruta del directorio a borrar.(tiene que estar vacio)
		     */
void renombrarArchivo(char* rutaDeArchivo,char* nuevoNombre);
/*
		   	 Parametros: - Ruta de Archivo
		   	   	   	   	 - Nombre nuevo

		   */
void listarArchivos(char* rutaDirectorio);
/*
 * Parametros: - Ruta directorio a nombrar.
 *
 */


void copiarArchivo(char* rutaArchivo, char* rutaCopia);
/*
	Parametros: Preguntar al hombre del oeste


*/

void truncarArchivo(char* rutaArchivo, int cantidadDeBytes);
/*
	Parametros: -Preguntar al hombre del oeste

*/
void leerArchivoCompleto(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer);

void moverArchivo(char* rutaOrigen, char* rutaDestino);

//OPERACIONES SECUNDARIAS//

void mapearEstructura(void* discoMapeado);
void mapearBloquesDeDatos(FILE* archivoAbierto);
void mapearTablaDeAsignaciones(FILE* archivoAbierto);
void mapearTablaDeArchivos(FILE* archivoAbierto);
void mapearBitmap(FILE* archivoAbierto);
void mapearHeader(FILE* archivoAbierto);

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
int directorioPadrePosicion(char* rutaAbsolutaArchivo);
int revisarMismoNombre(osada_file archivoARenombrar, char* nuevoNombre);
int posicionArchivoPorRuta(char* rutaAbsolutaArchivo);
int contarCantidadDeDirectorios();
void inicializarSemaforos();

void borrarDirectoriosVacios();
char* nombreDeRutaNueva(char* rutaDeArchivoNuevo);
char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo);

void escucharOperaciones(int* socketServer);
int cantidadDeBloquesVacios();
int ultimaPosicionBloqueDeDatos(osada_file archivo);

void eliminarUltimoBloqueDeArchivo(int posicionArchivoATruncar);
void borrarBloqueDeDatosEnElBitmap(int posicionBloque);

//CONEXIONES//
void startServer();
void clienteNuevo(void* parametro);

typedef struct{
	osada_header header;
	t_bitarray* bitmap;
	osada_file tablaDeArchivos[2048];
	int* tablaDeAsignaciones;
	osada_block* bloquesDeDatos;
}osada_bloqueCentral;


typedef struct{
	int puerto;
	char* ip;
}t_conexion;

typedef struct{
	osada_file_state state;
	char* nombreDeArchivo;


}t_osadaState;

osada_bloqueCentral* disco;
int tamanioDisco;
char* rutaDisco;
t_conexion conexion;


#endif /* POKEDEXSERVER_H_ */


