/*
 * entrenador.h
 *
 */

#ifndef ENTRENADOR1_ENTRENADOR_H_
#define ENTRENADOR1_ENTRENADOR_H_

#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/queue.h"
#include "conexiones.h"
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

#include "pkmn/battle.h"
#include <pkmn/factory.h>

void recorrerEPrintearLista(t_list* unaLista);

typedef struct datosRespuesta{
	int operacion;
	int pid;
}t_respuesta;

void serializarRespuesta(t_respuesta respuesta, char** buffer);

typedef struct {
	char* nombreMapa;
	char** objetivos;
	char* ip;
	int puerto;
} t_mapa;

typedef struct {
	char simbolo;
	char* nombre;
	char* rutaPokedex;
	int cantVidas;
	t_list* hojaDeViaje;
	int mapaActual;
	int posicion[2];
	char objetivoActual;
} t_entrenador;

t_list** pokemonesCapturados;
t_list** contextoPokemons;

//Logger
t_log* logEntrenador;

//Configuracion
t_entrenador entrenador ;
t_mapa mapa;

//Variables globales
int socketMapa = 0;
int posObjX = 0;
int posObjY = 0;
int cantDeadLocks = 0;
int tiempoBloqueadoEnPokeNests = 0;
int cantMuertes = 0;
int reintentos = 0;
int abandonar = -1;
bool alternateFlag = false;
bool esMiTurno = false;
bool cumpliObjetivos = false;
bool volverAlMismoMapa = false;
bool flagAtrapar = false;

//Obtiene los datos desde la metada del entrenador
void getMetadataEntrenador();
void inicializarEntrenador();

void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY);
void avanzarHastaPokenest(int posicionXPokenest, int posicionYPokenest);

void solicitarUbicacionPokenest(int* posx, int* posy, int index);
void chequearObjetivos(char pokemon);
void muerteDelEntrenador();
void recorrerEPrintearLista(t_list* unaLista);
void atraparUnPokemon(char pokemon);
void capturarPokemon();

void procesarRecibir();
void enviarInfoAlMapa();
void interactuarConMapas();
void controladorDeSeniales(int signo);
void agregarVida();
void seniales();

void liberarRecursosCapturados();
t_pokemon* getPokemonMasFuerte();

void destruirPokemon();
void destruirContexto(t_contextoPokemon* contexto);

void crearListaPokemones();
void *inicializar(int tamanio);

void imprimirListasPokemones();
void imprimirObjetivos(t_mapa* mapa);
void imprimirTiempos(time_t comienzo);

void* leerArchivoYGuardarEnCadena(int* tamanioDeArchivo, char* nombreDelArchivo);
void guardarEnDirdeBill(char* nombreArchivo, int tamanioDeArchivo, char* textoArch);
void copiarMedallaDelMapa(char* nombreDelMapa);
void borrarArchivosEnDirDeBill();
void borrarMedallas();

void pruebaCrearYEscribir();
void pruebaLeer();
void pruebaBorrar();
void pruebaOpenDirYReadDir();

#endif /* ENTRENADOR1_ENTRENADOR_H_ */
