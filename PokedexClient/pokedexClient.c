#include "pokedexClient.h"

#include <asm-generic/errno-base.h>
#include <sys/socket.h>
#include <conexiones.h>
#include <fcntl.h>
#include <fuse/fuse.h>
#include <fuse/fuse_common.h>
#include <fuse/fuse_compat.h>
#include <fuse/fuse_opt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>


t_log * myLog;

/*
 * Este es el nombre del archivo que se va a encontrar dentro de nuestro FS
 */


/*
 * Este es el path de nuestro, relativo al punto de montaje, archivo dentro del FS


/*
 * Esta es una estructura auxiliar utiliz
 * ada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */
struct t_runtime_options {
	char* welcome_msg;
} runtime_options;

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		stbuf - Esta esta estructura es la que debemos completar
 *
 * 	@RETURN
 * 		O archivo/directorio fue encontroado. -ENOENT archivo/directorio no encontrado
 */
static int fuseGetattr(const char *path, struct stat *stbuf) {
	int res = 0;
	log_trace(myLog, "Se quiso obtener la metadata de %s", path);
	memset(stbuf, 0, sizeof(struct stat));


	printf("La ruta que quiere leer el geTATTR es: %s\n", path);

	//Reservo memoria para la estructura que voy a enviar


	t_MensajeAtributosArchivoPokedexClient_PokedexServer* sendInfo = malloc(
			sizeof(t_MensajeAtributosArchivoPokedexClient_PokedexServer));

	//Cargo los contenidos que voy a enviar
	int tamanioRuta = string_length(path) + 1;
	sendInfo->tamanioRuta = tamanioRuta;
	sendInfo->rutaArchivo = malloc(tamanioRuta);
	memcpy(sendInfo->rutaArchivo, path, tamanioRuta);

	//Obtengo el tamaño de lo que voy a enviar
	size_t tamanioBuffer = sendInfo->tamanioRuta + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(int) * 2);
	operacionBuilder(tamanioBuffer, pedido, ATRIBUTO_ARCHIVO);

	void* operacionARealizar = malloc(sizeof(int) * 2);

	serializarOperaciones(operacionARealizar, pedido);
	enviar(&socketServer, operacionARealizar, sizeof(int) * 2);
	printf("Numero de operacion: %i \n", pedido->operacion);
	//printf("%i \n", pedido->tamanioBuffer);

	void* bufferSerializado;
	bufferSerializado = malloc(tamanioBuffer);
	serializarMensajeAtributosArchivo(bufferSerializado, sendInfo);
	enviar(&socketServer, bufferSerializado, pedido->tamanioBuffer);
	t_MensajeAtributosArchivoPokedexClient_PokedexServer* deserializadoSeniora;
	deserializadoSeniora = malloc(
			sizeof(t_MensajeAtributosArchivoPokedexClient_PokedexServer));
	deserializarMensajeAtributosArchivo(bufferSerializado,
			deserializadoSeniora);
	if(strcmp(deserializadoSeniora->rutaArchivo,"/") ==1) printf("Ruta del archivo: %s \n", deserializadoSeniora->rutaArchivo);
	//printf("%d \n", deserializadoSeniora->tamanioRuta);
	t_MensajeAtributosArchivoPokedexServer_PokedexClient* atributosArchivo =
			malloc(
					sizeof(t_MensajeAtributosArchivoPokedexServer_PokedexClient));
	void* buffer = malloc(sizeof(t_MensajeAtributosArchivoPokedexServer_PokedexClient));
	recibir(&socketServer, buffer, sizeof(t_MensajeAtributosArchivoPokedexServer_PokedexClient));
	deserializarAtributos(buffer, atributosArchivo);
	free(bufferSerializado);
	free(deserializadoSeniora);
	free(sendInfo->rutaArchivo);
	free(sendInfo);
	free(pedido);
	free(buffer);
	free(operacionARealizar);
	if (atributosArchivo->estado == -1) {
		free(atributosArchivo);

		return -ENOENT;
	}

	if (strcmp(path, "/") == 0) {

		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;

	} else {

		if (atributosArchivo->estado == 1) {
			stbuf->st_mode = S_IFREG | 0777;
			stbuf->st_nlink = 1;
			stbuf->st_size = atributosArchivo->tamanio;
			stbuf->st_mtim = atributosArchivo->ts;


		}	else	{

			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
		}

	}

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje

	/*	if (strcmp(path, "/") == 0) {

	 } else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {


	 stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
	 } else {
	 res = -ENOENT;
	 }*/
	free(atributosArchivo);

	return res;
}

t_MensajeListarArchivosPokedexClient_PokedexServer* readDirInfoBuilder(
		const char* path) {

	int tamanioPath = string_length(path) + 1;
	int tamanioStruct = tamanioPath + sizeof(int);

	t_MensajeListarArchivosPokedexClient_PokedexServer* sendInfo = malloc(
			tamanioStruct);
	sendInfo->rutaDeArchivo = path;
	sendInfo->tamanioRuta = strlen(sendInfo->rutaDeArchivo) + 1;
	return sendInfo;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es un buffer donde se colocaran los nombres de los archivos y directorios
 * 		      que esten dentro del directorio indicado por el path
 * 		filler - Este es un puntero a una función, la cual sabe como guardar una cadena dentro
 * 		         del campo buf
 *
 * 	@RETURN
 * 		O directorio fue encontrado. -ENOENT directorio no encontrado
 */
static int fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {

	t_MensajeListarArchivosPokedexClient_PokedexServer* sendInfo =
			readDirInfoBuilder(path);
	size_t tamaniobuffer = sendInfo->tamanioRuta + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(int) * 2);

	operacionBuilder(tamaniobuffer, pedido, LISTAR_DIRECTORIO);

	void* operacionARealizar = malloc(sizeof(int) * 2);

	serializarOperaciones(operacionARealizar, pedido);
	enviar(&socketServer, operacionARealizar, sizeof(int) * 2);
	printf("Numero de operacion: %i \n", pedido->operacion);
	//printf("%i \n", pedido->tamanioBuffer);

	void* bufferSerializado;
	bufferSerializado = malloc(tamaniobuffer);
	serializarMensajeListarArchivos(bufferSerializado, sendInfo);
	enviar(&socketServer, bufferSerializado, pedido->tamanioBuffer);
	t_MensajeListarArchivosPokedexClient_PokedexServer* deserializadoSeniora;
	deserializadoSeniora = malloc(
			sizeof(t_MensajeListarArchivosPokedexClient_PokedexServer));
	deserializarMensajeListarArchivos(bufferSerializado, deserializadoSeniora);
	if(strcmp(deserializadoSeniora->rutaDeArchivo,"/") ==1) printf("Ruta de directorio: %s \n", deserializadoSeniora->rutaDeArchivo);
	//printf("%d \n", deserializadoSeniora->tamanioRuta);
	free(bufferSerializado);
	free(deserializadoSeniora);
	int tamanioLista;
	recibir(&socketServer, &tamanioLista, sizeof(int));
	if (tamanioLista == 0) {
		free(pedido);
		free(operacionARealizar);
		free(sendInfo);

		return 0;
	}
	char* bufferDeListar = malloc(tamanioLista);
	recibir(&socketServer, bufferDeListar, tamanioLista);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	int ciclosLista = tamanioLista / 18;

	char* entrante = malloc(18);
	int i;
	for (i = 0; i < ciclosLista; i++) {

		memcpy(entrante, bufferDeListar + i * 18, 18);
		printf(" %s \n", entrante);
		filler(buf, entrante, NULL, 0);

	}
	//filler(buf, bufferDeListar, NULL, 0);
	/*
	 (void) offset;
	 (void) fi;
	 log_trace(myLog, "Se quiso obtener la lista de los archivos en %s", path);
	 if (strcmp(path, "/") != 0)
	 return -ENOENT;
	 */
	// "." y ".." son entradas validas, la primera es una referencia al directorio donde estamos parados
	// y la segunda indica el directorio padre
	/*	filler(buf, ".", NULL, 0);
	 filler(buf, "..", NULL, 0);
	 filler(buf, DEFAULT_FILE_NAME, NULL, 0);*/

	free(pedido);
	free(entrante);
	free(bufferDeListar);
	free(operacionARealizar);
	free(sendInfo);

	return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para tratar de abrir un archivo
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		fi - es una estructura que contiene la metadata del archivo indicado en el path
 *
 * 	@RETURN
 * 		O archivo fue encontrado. -EACCES archivo no es accesible
 */
static int fuseOpen(const char *path, struct fuse_file_info *fi) {
	log_trace(myLog, "Se abrio %s", path);
	/*	if (strcmp(path, DEFAULT_FILE_PATH) != 0)
	 return -ENOENT;

	 if ((fi->flags & 3) != O_RDONLY)
	 return -EACCES;*/

	return 0;
}






void operacionBuilder(size_t tamaniobuffer, t_pedidoPokedexCliente* pedido,
		enum_operacion oper) {
	pedido->operacion = oper;
	pedido->tamanioBuffer = tamaniobuffer;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener el contenido de un archivo
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es el buffer donde se va a guardar el contenido solicitado
 * 		size - Nos indica cuanto tenemos que leer
 * 		offset - A partir de que posicion del archivo tenemos que leer
 *
 * 	@RETURN
 * 		Si se usa el parametro direct_io los valores de retorno son 0 si  elarchivo fue encontrado
 * 		o -ENOENT si ocurrio un error. Si el parametro direct_io no esta presente se retorna
 * 		la cantidad de bytes leidos o -ENOENT si ocurrio un error. ( Este comportamiento es igual
 * 		para la funcion write )
 */
static int fuseRead(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	int tamanioRuta = string_length(path) + 1;

	t_MensajeLeerPokedexClient_PokedexServer* sendInfo = malloc(
			sizeof(t_MensajeLeerPokedexClient_PokedexServer));

	sendInfo->cantidadDeBytes = size;

	sendInfo->offset = offset;

	sendInfo->rutaArchivo = malloc(tamanioRuta);
	memcpy(sendInfo->rutaArchivo, path, tamanioRuta);

	sendInfo->tamanioRuta = tamanioRuta;

	size_t tamaniobuffer = sendInfo->tamanioRuta + sizeof(int) * 3;
	t_pedidoPokedexCliente* pedido = malloc(sizeof(int) * 2);

	operacionBuilder(tamaniobuffer, pedido, LEER_ARCHIVO);

	void* operacionARealizar = malloc(sizeof(int) * 2);

	serializarOperaciones(operacionARealizar, pedido);
	enviar(&socketServer, operacionARealizar, sizeof(int) * 2);

	free(operacionARealizar);

	void* bufferSerializado;
	bufferSerializado = malloc(tamaniobuffer);
	serializarMensajeLeerArchivo(bufferSerializado, sendInfo);
	enviar(&socketServer, bufferSerializado, pedido->tamanioBuffer);

	free(pedido);
	log_trace(myLog, "Se quiso leer %s", path);

	void* bufferRespuesta = malloc(sizeof(int) * 2);
	t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

	recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
	deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
	free(bufferRespuesta);

	char *buffer = malloc(size);
	recibir(&socketServer, buffer, sendInfo->cantidadDeBytes);

	if (respuesta->resultado == ERROR_ACCESO) {
		free(sendInfo->rutaArchivo);
		free(sendInfo);
		free(buffer);
		free(respuesta);
		return -ENOENT;

	} else if (respuesta->resultado == ERROR_VACIO) {

	} else {

		memcpy(buf, buffer, size);

	}
	/*
	 (void) fi;


	 size_t len;


	 if (strcmp(path, sendInfo->rutaArchivo) != 0)
	 return -ENOENT;

	 len = strlen(sendInfo->buffer);
	 if (offset < len) {
	 if (offset + size > len)
	 size = len - offset;


	 memcpy(buf,sendInfo->buffer + offset,size);
	 } else
	 size = 0;
	 */

	int tamanio = respuesta->tamanio;

	free(sendInfo->rutaArchivo);
	free(sendInfo);
	free(respuesta);
	free(buffer);
	return tamanio;

}

static int fuseWrite(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi) {

	//Seteo las estructuras para serializar en un envio

	int tamanioRuta = strlen(path) + 1;

	t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoEnvio = malloc(
			sizeof(t_MensajeEscribirArchivoPokedexClient_PokedexServer));

	infoEnvio->offset = offset;
	infoEnvio->cantidadDeBytes = size;
	infoEnvio->tamanioRuta = tamanioRuta;

	infoEnvio->bufferAEscribir = malloc(size);
	memcpy(infoEnvio->bufferAEscribir, buf, size);

	infoEnvio->rutaArchivo = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaArchivo, path, tamanioRuta);

	int tamanioBufferEscritura = tamanioRuta + size + sizeof(int) * 3;

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));

	pedido->operacion = ESCRIBIR_ARCHIVO;
	pedido->tamanioBuffer = tamanioBufferEscritura;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferEscritura = malloc(tamanioBufferEscritura);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeEscribirOModificarArchivo(bufferEscritura, infoEnvio);

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferEscritura, tamanioBufferEscritura);

	free(bufferOperacion);
	free(bufferEscritura);
	free(infoEnvio->bufferAEscribir);
	free(infoEnvio->rutaArchivo);
	free(infoEnvio);
	free(pedido);


	//Recibir respuesta
	void* bufferRespuesta = malloc(sizeof(int) * 2);
	t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

	recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
	deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
	free(bufferRespuesta);


	if(respuesta->resultado == NOHAYESPACIO){


		return -ENOSPC;
	}



	 if(size!=respuesta->tamanio){



	 return -ENOENT;

	 }

	free(respuesta);

	return size;
}

static int fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fi) {

	//Seteo estructuras de envio

	t_MensajeCrearArchivoPokedexClient_PokedexServer* infoEnvio = malloc(
			sizeof(t_MensajeCrearArchivoPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;

	infoEnvio->tamanioRuta = tamanioRuta;

	infoEnvio->rutaDeArchivoACrear = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaDeArchivoACrear, path, tamanioRuta);

	int tamanioBuffer = tamanioRuta + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = CREAR_ARCHIVO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferCreacion = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeCrearArchivo(bufferCreacion, infoEnvio);

	//Envio al servidor

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferCreacion, tamanioBuffer);

	//Recibir respuesta
	void* bufferRespuesta = malloc(sizeof(t_RespuestaPokedexCliente));
	t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

	recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
	deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
	free(bufferRespuesta);

	//Procesar respuesta
	if(respuesta->resultado == MISMO_NOMBRE){
		free(respuesta);
		free(bufferCreacion);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDeArchivoACrear);
		free(infoEnvio);
		return -EEXIST;

	}
	if (respuesta->resultado == ERRORENAMETOOLONG){

		free(respuesta);
		free(bufferCreacion);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDeArchivoACrear);
		free(infoEnvio);
		return -ENAMETOOLONG;
	}
	if(respuesta->resultado == ERROREDQUOT){
			free(respuesta);
			free(bufferCreacion);
			free(bufferOperacion);
			free(pedido);
			free(infoEnvio->rutaDeArchivoACrear);
			free(infoEnvio);
			return -EDQUOT;

		}
	if(respuesta->resultado == ERROR_ACCESO){
				free(respuesta);
				free(bufferCreacion);
				free(bufferOperacion);
				free(pedido);
				free(infoEnvio->rutaDeArchivoACrear);
				free(infoEnvio);
				return -EACCES;

			}

	//Libero todas las estructuras
	free(respuesta);
	free(bufferCreacion);
	free(bufferOperacion);
	free(pedido);
	free(infoEnvio->rutaDeArchivoACrear);
	free(infoEnvio);

	return 0;

}

static int fusemkdir(const char *path, mode_t mode) {

	//Seteo estructuras de envio

	t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoEnvio = malloc(
			sizeof(t_MensajeCrearDirectorioPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;

	infoEnvio->tamanioRuta = tamanioRuta;

	infoEnvio->rutaDirectorioPadre = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaDirectorioPadre, path, tamanioRuta);

	int tamanioBuffer = tamanioRuta + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = CREAR_DIRECTORIO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferCreacionDirectorio = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeCrearDirectorio(bufferCreacionDirectorio, infoEnvio);

	//Envio al servidor

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferCreacionDirectorio, tamanioBuffer);


	//Recibir respuesta
		void* bufferRespuesta = malloc(sizeof(t_RespuestaPokedexCliente));
		t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

		recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
		deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
		free(bufferRespuesta);

	//Procesar respuesta
		if(respuesta->resultado == MISMO_NOMBRE){
		free(respuesta);
		free(bufferCreacionDirectorio);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDirectorioPadre);
		free(infoEnvio);

		return -EEXIST;

		}
		if (respuesta->resultado == ERRORENAMETOOLONG){

		free(respuesta);
		free(bufferCreacionDirectorio);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDirectorioPadre);
		free(infoEnvio);
		return -ENAMETOOLONG;
		}
		if (respuesta->resultado == ERROR_ACCESO){

				free(respuesta);
				free(bufferCreacionDirectorio);
				free(bufferOperacion);
				free(pedido);
				free(infoEnvio->rutaDirectorioPadre);
				free(infoEnvio);
				return -EACCES;
		}
		if(respuesta->resultado == ERROREDQUOT){
					free(respuesta);
					free(bufferCreacionDirectorio);
					free(bufferOperacion);
					free(pedido);
					free(infoEnvio->rutaDirectorioPadre);
					free(infoEnvio);
					return -EDQUOT;

				}
		//Libero todas las estructuras

		free(respuesta);
		free(bufferCreacionDirectorio);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDirectorioPadre);
		free(infoEnvio);

	return 0;

}

static int fusermdir(const char *path) {

	//Seteo estructuras de envio

	t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoEnvio =
			malloc(
					sizeof(t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;

	infoEnvio->tamanioRuta = tamanioRuta;

	infoEnvio->rutaDirectorioABorrar = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaDirectorioABorrar, path, tamanioRuta);

	int tamanioBuffer = tamanioRuta + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = BORRAR_DIRECTORIO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferBorrarDirectorio = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeBorrarDirectorio(bufferBorrarDirectorio, infoEnvio);

	//Envio al servidor

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferBorrarDirectorio, tamanioBuffer);

	//Recibir respuesta
	void* bufferRespuesta = malloc(sizeof(t_RespuestaPokedexCliente));
	t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

	recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
	deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
	free(bufferRespuesta);

	if(respuesta->resultado == ERROR_NO_VACIO){
		free(respuesta);
		free(bufferBorrarDirectorio);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDirectorioABorrar);
		free(infoEnvio);

		return -ENOTEMPTY;
	}
	if(respuesta->resultado == ERROR_ACCESO){
			free(respuesta);
			free(bufferBorrarDirectorio);
			free(bufferOperacion);
			free(pedido);
			free(infoEnvio->rutaDirectorioABorrar);
			free(infoEnvio);

			return -EACCES;
		}



	//Libero todas las estructuras
	free(bufferBorrarDirectorio);
	free(bufferOperacion);
	free(pedido);
	free(infoEnvio->rutaDirectorioABorrar);
	free(infoEnvio);
	free(respuesta);

	return 0;
}

static int fuseDelete(const char *path) {

	//Seteo estructuras de envio

	t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoEnvio = malloc(
			sizeof(t_MensajeBorrarArchivoPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;

	infoEnvio->tamanioRuta = tamanioRuta;

	infoEnvio->rutaArchivoABorrar = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaArchivoABorrar, path, tamanioRuta);

	int tamanioBuffer = tamanioRuta + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = BORRAR_ARCHIVO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferBorrarArchivo = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeBorrarArchivo(bufferBorrarArchivo, infoEnvio);

	//Envio al servidor

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferBorrarArchivo, tamanioBuffer);

	//Recibir respuesta
	void* bufferRespuesta = malloc(sizeof(t_RespuestaPokedexCliente));
	t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

	recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
	deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
	free(bufferRespuesta);


	if(respuesta->resultado == ERROR_ACCESO){
		free(bufferBorrarArchivo);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaArchivoABorrar);
		free(infoEnvio);
		free(respuesta);

		return -EACCES;
	}



	//Libero todas las estructuras
	free(bufferBorrarArchivo);
	free(bufferOperacion);
	free(pedido);
	free(infoEnvio->rutaArchivoABorrar);
	free(infoEnvio);
	free(respuesta);
	return 0;
}

static int fuseMove(const char* path, const char *newPath) {

	//Seteo estructuras de envio

	t_MensajeMoverArchivoPokedexClient_PokedexServer* infoEnvio = malloc(
			sizeof(t_MensajeMoverArchivoPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;
	int tamanioRutaNueva = strlen(newPath) + 1;

	infoEnvio->tamanioRuta = tamanioRuta;

	infoEnvio->tamanioNuevaRuta = tamanioRutaNueva;

	infoEnvio->rutaDeArchivo = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaDeArchivo, path, tamanioRuta);

	infoEnvio->nuevaRuta = malloc(tamanioRutaNueva);
	memcpy(infoEnvio->nuevaRuta, newPath, tamanioRutaNueva);

	int tamanioBuffer = tamanioRutaNueva + tamanioRuta + sizeof(int) * 2;

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = MOVER_ARCHIVO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferMoverArchivo = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeMoverArchivo(bufferMoverArchivo, infoEnvio);

	//Envio al servidor

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferMoverArchivo, tamanioBuffer);

	//Recibir respuesta
			void* bufferRespuesta = malloc(sizeof(t_RespuestaPokedexCliente));
			t_RespuestaPokedexCliente* respuesta = malloc(sizeof(int) * 2);

			recibir(&socketServer, bufferRespuesta, sizeof(int) * 2);
			deserializarRespuestaOperaciones(bufferRespuesta, respuesta);
			free(bufferRespuesta);

	//Procesar respuesta
	if(respuesta->resultado == MISMO_NOMBRE){
			free(respuesta);
			free(bufferMoverArchivo);
			free(bufferOperacion);
			free(pedido);
			free(infoEnvio->rutaDeArchivo);
			free(infoEnvio->nuevaRuta);
			free(infoEnvio);

			return -EEXIST;

			}
			if (respuesta->resultado == ERRORENAMETOOLONG){

			free(respuesta);
			free(bufferMoverArchivo);
			free(bufferOperacion);
			free(pedido);
			free(infoEnvio->rutaDeArchivo);

			free(infoEnvio->nuevaRuta);
			free(infoEnvio);
			return -ENAMETOOLONG;
			}
			if(respuesta->resultado == ERROR_ACCESO){
						free(respuesta);
						free(bufferMoverArchivo);
						free(bufferOperacion);
						free(pedido);
						free(infoEnvio->rutaDeArchivo);
						free(infoEnvio->nuevaRuta);
						free(infoEnvio);

						return -EACCES;

						}



	//Libero todas las estructuras
	free(bufferMoverArchivo);
	free(bufferOperacion);
	free(pedido);
	free(infoEnvio->rutaDeArchivo);
	free(infoEnvio->nuevaRuta);
	free(infoEnvio);
	free(respuesta);

	return 0;

}

static int fuseTruncate(const char *path, off_t offset) {

	//Seteo estructuras de envio

	t_MensajeTruncarArchivoPokedexClient_PokedexServer* infoEnvio = malloc(
			sizeof(t_MensajeTruncarArchivoPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;

	infoEnvio->tamanioRuta = tamanioRuta;
	infoEnvio->nuevoTamanio = offset;

	infoEnvio->rutaDeArchivo = malloc(tamanioRuta);
	memcpy(infoEnvio->rutaDeArchivo, path, tamanioRuta);

	int tamanioBuffer = tamanioRuta + sizeof(int) + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = TRUNCAR_ARCHIVO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(int) * 2);
	void* bufferTruncarArchivo = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeTruncarArchivo(bufferTruncarArchivo, infoEnvio);

	//Envio al servidor

	enviar(&socketServer, bufferOperacion, sizeof(int) * 2);
	enviar(&socketServer, bufferTruncarArchivo, tamanioBuffer);

	//Respuesta del servidor
	void* bufferRespuesta = malloc(sizeof(int));
	int respuesta = 0;
	recibir(&socketServer, bufferRespuesta, sizeof(int));
	memcpy(&respuesta, bufferRespuesta, sizeof(int));
	free(bufferRespuesta);

	if(respuesta == MUYGRANDE){
		free(bufferTruncarArchivo);
		free(bufferOperacion);
		free(pedido);
		free(infoEnvio->rutaDeArchivo);
		free(infoEnvio);

		return -EFBIG;
	}


	//Libero todas las estructuras
	free(bufferTruncarArchivo);
	free(bufferOperacion);
	free(pedido);
	free(infoEnvio->rutaDeArchivo);
	free(infoEnvio);

	return 0;

}

static int fuseUtimens(const char *path, const struct timespec tv[2]) {


	t_MensajeUtimensPokedexClient_PokedexServer* infoAenviar = malloc(
			sizeof(t_MensajeUtimensPokedexClient_PokedexServer));

	int tamanioRuta = strlen(path) + 1;
	infoAenviar->path = malloc(tamanioRuta);
	memcpy(infoAenviar->path, path, tamanioRuta);
	infoAenviar->tamanioRuta = tamanioRuta;

	memmove(infoAenviar->tv, tv, sizeof(infoAenviar->tv));

	int tamanioBuffer = tamanioRuta + sizeof(infoAenviar->tv) + sizeof(int);

	t_pedidoPokedexCliente* pedido = malloc(sizeof(t_pedidoPokedexCliente));
	pedido->operacion = UTIMENS_ARCHIVO;
	pedido->tamanioBuffer = tamanioBuffer;

	void* bufferOperacion = malloc(sizeof(t_pedidoPokedexCliente));
	void* bufferUtimensArchivo = malloc(tamanioBuffer);

	serializarOperaciones(bufferOperacion, pedido);
	serializarMensajeUtimensArchivo(bufferUtimensArchivo, infoAenviar);

	enviar(&socketServer, bufferOperacion, sizeof(t_pedidoPokedexCliente));
	enviar(&socketServer, bufferUtimensArchivo, tamanioBuffer);


	free(bufferUtimensArchivo);
	free(bufferOperacion);
	free(pedido);
	free(infoAenviar->path);
	free(infoAenviar);


	return 0;
}

static int fuseChmod(const char *path, struct fuse_file_info *fi) {

	return 0;
}

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations fuseOper = {

		.getattr  = fuseGetattr,
		.readdir  = fuseReaddir,
		.open 	  = fuseOpen,
		.read 	  = fuseRead,
		.write    = fuseWrite,
		.create   = fuseCreate,
		.mkdir    = fusemkdir,
		.rmdir    = fusermdir,
		.unlink   = fuseDelete,
		.truncate = fuseTruncate,
		.rename   = fuseMove,
		.chmod    = fuseChmod,
		.chown    = fuseChmod,
		.utime    = fuseChmod,
		.utimens  = fuseUtimens,
		.flush    = fuseChmod,
		.statfs   = fuseChmod

};

/** keys for FUSE_OPT_ options */
enum {
	KEY_VERSION, KEY_HELP,
};

/*
 * Esta estructura es utilizada para decirle a la biblioteca de FUSE que
 * parametro puede recibir y donde tiene que guardar el valor de estos
 */
static struct fuse_opt fuse_options[] = {
// Este es un parametro definido por nosotros
		CUSTOM_FUSE_OPT_KEY("--welcome-msg %s", welcome_msg, 0),

		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END, };

int openConnection(char* ipServer, char* puerto) {
	socket(miSocket, SOCK_STREAM, AF_INET);

	if(ipServer == '\0' || puerto == '\0'){
		printf("Se deben setear las variables de ambiente\n");
		return EXIT_FAILURE;
	}

	conexion.puerto = atoi(puerto);

	conexion.ip = ipServer;



	socketServer = conectarseA(conexion.ip, conexion.puerto);

	return 0;

}

// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS

int main(int argc, char *argv[]) {

	myLog = log_create("Log", "Fuse", 0, 0);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	//Abro la conexion con variables de entorno
	if(openConnection(getenv("IPSERVER"),getenv("PUERTOSERVER"))){
		return EXIT_FAILURE;
	}

	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));


	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1) {
		// error parsing options //
		perror("Invalid arguments!");
		return EXIT_FAILURE;

	}



	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	return fuse_main(args.argc, args.argv, &fuseOper, NULL);

}







