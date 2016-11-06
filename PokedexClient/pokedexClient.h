/*
 * pokedexClient.h
 *
 */

#ifndef POKEDEXCLIENT_H_
#define POKEDEXCLIENT_H_

#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <conexiones.h>

typedef struct{
	int puerto;
	char* ip;
}t_conexion;

typedef struct{
	const char *path;
	char *buf;
	size_t size;
	off_t offset;

}t_read_fuse;

t_conexion conexion;
int socketServer;
int miSocket;
#endif /* POKEDEXCLIENT_H_ */

typedef enum{
	LEER_ARCHIVO = 0,
	CREAR_ARCHIVO,
	ESCRIBIR_ARCHIVO,
	BORRAR_ARCHIVO,
	CREAR_DIRECTORIO,
	BORRAR_DIRECTORIO,
	RENOMBRAR_ARCHIVO,
	LISTAR_DIRECTORIO,
	TRUNCAR_ARCHIVO,
	MOVER_ARCHIVO,
	ATRIBUTO_ARCHIVO
} enum_operacion;


char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo);


void operacionBuilder(size_t tamaniobuffer, t_pedidoPokedexCliente* pedido,
		enum_operacion oper);




