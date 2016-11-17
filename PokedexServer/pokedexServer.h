/*
 * pokedexServer.h
 *
 */

#ifndef POKEDEXSERVER_H_
#define POKEDEXSERVER_H_

#define ULTIMO_BLOQUE -1
#define BLOQUE_VACIO 2,147,483,647
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
	MOVER_ARCHIVO,
	ATRIBUTO_ARCHIVO
} enum_operacion;

typedef enum{
	SUCCESSFUL_EXIT = 0,
	ERROR_ACCESO,
	ERROR_VACIO

} enum_errores;


#include "conexiones.h"


int tamanioFileSystem;
char* RUTA_DISCO;
sem_t semaforos_permisos[2048];
sem_t semaforoTruncar;
sem_t semaforoRoot;
sem_t semaforoTablaArchivos;


//OPERACIONES PRINCIPALES//


int leerArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer, int* socket);
/* Parametros
		   	   	   - Buffer(contenido real a leer),
		   	   	   - Ruta del Archivo,
		   	   	   - Offset (Punto de arranque),
		   	   	   - Tamaño del buffer.

		   	*/

int crearArchivo(char* rutaArchivoNuevo, int* socket);
		/*Parametros:
		    		- Ruta del archivo (se separa el ultimo parametro para obtener el nombre del archivo)

		    */
int escribirOModificarArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer,int* socket);
/*Parametros
		   		   	   	   - Buffer(contenido real a escribir),
		   		   	   	   - Ruta del Archivo,
		   		   	   	   - Offset (Punto de arranque),
		   		   	   	   - Tamaño del buffer.
		   */
int borrarArchivos(char* rutaDeArchivo, int* socket);
/*Parametros
		   		   	   	   - Ruta del Archivo
		   */
int crearDirectorio(char* rutaDirectorioPadre, int* socket);
/*Parametros
		   	   	   	   	   - Ruta del directorio a crear.
		   */
int borrarDirectoriosVacios();

int borrarDirectorioVacios(char* rutaDelDirectorioABorrar, int* socket);
/*Parametros:
		     	 	 	 - Ruta del directorio a borrar.(tiene que estar vacio)
		     */
int renombrarArchivo(char* rutaDeArchivo,char* nuevoNombre, int* socket);
/*
		   	 Parametros: - Ruta de Archivo
		   	   	   	   	 - Nombre nuevo

		   */
int listarArchivos(char* rutaDirectorio, int* socketEnvio);
/*
 * Parametros: - Ruta directorio a nombrar.
 *
 */


int copiarArchivo(char* rutaArchivo, char* rutaCopia, int* socket);
/*
	Parametros: Preguntar al hombre del oeste


*/

int truncarArchivo(char* rutaArchivo, int cantidadDeBytes);
/*
	Parametros: -Preguntar al hombre del oeste

*/

int moverArchivo(char* rutaOrigen, char* rutaDestino, int* socket);

int atributosArchivo(char* rutaArchivo, int* socket);

//OPERACIONES SECUNDARIAS//
int strcontains(char* cadena1, char* cadena2);
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
t_list* crearListaDeSecuencia(osada_file archivo);
//void borrarDirectoriosVacios();
char* nombreDeRutaNueva(char* rutaDeArchivoNuevo);
char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo);
void destruirEntero(int* puntero);
void escucharOperaciones(int* socketServer);
int cantidadDeBloquesVacios();
int ultimaPosicionBloqueDeDatos(osada_file archivo);
int string_count(char* unaCadena);
t_list* listaDeSecuenciaDeBloques(int posicionArchivo);
void persistirEstructura(void* discoMapeado);
void eliminarUltimoBloqueDeArchivo(int posicionArchivoATruncar);
void borrarBloqueDeDatosEnElBitmap(int posicionBloque);
void persistirDisco(void* discoMapeado, FILE* archivo);


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

void destruirDisco(osada_bloqueCentral* discoADestruir);

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
t_log* logPokedex;

#endif /* POKEDEXSERVER_H_ */


