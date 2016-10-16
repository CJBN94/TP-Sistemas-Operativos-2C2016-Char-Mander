/*
 * pokedexClient.h
 *
 */

#ifndef POKEDEXCLIENT_H_
#define POKEDEXCLIENT_H_

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
#endif /* POKEDEXCLIENT_H_ */
