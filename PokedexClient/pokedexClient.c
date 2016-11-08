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
/* Este es el contenido por defecto que va a contener
 * el unico archivo que se encuentre presente en el FS.
 * Si se modifica la cadena se podra ver reflejado cuando
 * se lea el contenido del archivo
 */
#define DEFAULT_FILE_CONTENT "nombre=Red\nsimbolo=@\nhojaDeViaje=[PuebloPaleta,CiudadVerde,CiudadPlateada]\nobj[PuebloPaleta]=[P,B,G]\nobj[CiudadVerde]=[C,Z,C]\nobj[CiudadPlateada]=[P,M,P,M,S]\nvidas=5\nreintentos=0"

/*
 * Este es el nombre del archivo que se va a encontrar dentro de nuestro FS
 */
#define DEFAULT_FILE_NAME "metaDataEntrenador"

/*
 * Este es el path de nuestro, relativo al punto de montaje, archivo dentro del FS
 */
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

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

	int tamanioMensaje=strlen(path)+sizeof(int)+1;
	t_MensajeAtributosArchivoPokedexClient_PokedexServer* sendInfo=malloc(sizeof(t_MensajeAtributosArchivoPokedexClient_PokedexServer));

	int tamanioRuta = string_length(path)+1;
	sendInfo->tamanioRuta = tamanioRuta;

	sendInfo->rutaArchivo= malloc(tamanioRuta);
	memcpy(sendInfo->rutaArchivo,path,tamanioRuta);


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
		serializarMensajeAtributosArchivo(bufferSerializado,sendInfo);
		enviar(&socketServer, bufferSerializado, pedido->tamanioBuffer);
		t_MensajeAtributosArchivoPokedexClient_PokedexServer* deserializadoSeniora;
		deserializadoSeniora = malloc(
				sizeof(t_MensajeAtributosArchivoPokedexClient_PokedexServer));
		deserializarMensajeAtributosArchivo(bufferSerializado,deserializadoSeniora);
		printf("Ruta del archivo: %s \n", deserializadoSeniora->rutaArchivo);
		//printf("%d \n", deserializadoSeniora->tamanioRuta);
		t_MensajeAtributosArchivoPokedexServer_PokedexClient* atributosArchivo=malloc(sizeof(t_MensajeAtributosArchivoPokedexServer_PokedexClient));
		void* buffer=malloc(sizeof(int)*2);
		recibir(&socketServer,buffer,8);
		deserializarAtributos(buffer,atributosArchivo);
		free(bufferSerializado);
		free(deserializadoSeniora);
		free(sendInfo);
		free(pedido);
		free(buffer);
		free(operacionARealizar);
		if(atributosArchivo->estado == -1){
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

		} else {

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

	t_MensajeListarArchivosPokedexClient_PokedexServer* sendInfo = malloc(tamanioStruct);
	sendInfo->rutaDeArchivo = path;
	sendInfo->tamanioRuta = strlen(sendInfo->rutaDeArchivo)+1;
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
	printf("Ruta de directorio: %s \n", deserializadoSeniora->rutaDeArchivo);
	//printf("%d \n", deserializadoSeniora->tamanioRuta);
	free(bufferSerializado);
	free(deserializadoSeniora);
	int tamanioLista;
	recibir(&socketServer, &tamanioLista, sizeof(int));
	char* bufferDeListar = malloc(tamanioLista);
	recibir(&socketServer, bufferDeListar, tamanioLista);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	int ciclosLista = tamanioLista / 17;
	char* entrante = malloc(17);
	int i;
	for (i = 0; i < ciclosLista; i++) {

		memcpy(entrante, bufferDeListar + i * 17, 17);
		printf(" %s \n", entrante);
		filler(buf, entrante, NULL, 0);

	}
	//filler(buf, bufferDeListar, NULL, 0);

	(void) offset;
	(void) fi;
	/*	log_trace(myLog, "Se quiso obtener la lista de los archivos en %s", path);
	 if (strcmp(path, "/") != 0)
	 return -ENOENT;*/

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

void enviarEscribirConSerializadores(off_t offset, const char* path,
		size_t size, char* buf) {
	int tamanioDelBufferAEnviar =
			sizeof(t_MensajeLeerPokedexClient_PokedexServer);
	t_pedidoPokedexCliente* operacionYTamanio = malloc(
			sizeof(t_pedidoPokedexCliente));
	t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoAEnviar = malloc(
			tamanioDelBufferAEnviar);
	operacionYTamanio->operacion = ESCRIBIR_ARCHIVO;
	operacionYTamanio->tamanioBuffer = tamanioDelBufferAEnviar;
	void* operacionSerializada = malloc(sizeof(t_pedidoPokedexCliente));
	serializarOperaciones(operacionSerializada, operacionSerializada);
	infoAEnviar->offset = offset;
	infoAEnviar->rutaArchivo = path;
	infoAEnviar->tamanioRuta = string_length(infoAEnviar->rutaArchivo);
	infoAEnviar->bufferAEscribir = "Hola";
	infoAEnviar->cantidadDeBytes = string_length(infoAEnviar->bufferAEscribir);
	void* bufferSerializado = malloc(tamanioDelBufferAEnviar);
	serializarMensajeEscribirOModificarArchivo(bufferSerializado, infoAEnviar);
	enviar(&socketServer, operacionSerializada, sizeof(int) * 2);
	enviar(&socketServer, bufferSerializado, tamanioDelBufferAEnviar);
	recibir(&miSocket, buf, size);
}

t_MensajeLeerPokedexClient_PokedexServer* readInfoBuilder(off_t offset,
		const char* path, char* buf) {
	t_MensajeLeerPokedexClient_PokedexServer* infoAEnviar = malloc(1000);
	infoAEnviar->buffer = buf;
	infoAEnviar->cantidadDeBytes = strlen(infoAEnviar->buffer);
	infoAEnviar->offset = offset;
	infoAEnviar->rutaArchivo = path;
	infoAEnviar->tamanioRuta = string_length(infoAEnviar->rutaArchivo);
	return infoAEnviar;
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

	t_MensajeLeerPokedexClient_PokedexServer* sendInfo = malloc(sizeof(t_MensajeLeerPokedexClient_PokedexServer));

	sendInfo->buffer = malloc(size);

	sendInfo->cantidadDeBytes = size;

	sendInfo->offset = offset;

	sendInfo->rutaArchivo = malloc(tamanioRuta);
	memcpy(sendInfo->rutaArchivo,path,tamanioRuta);

	sendInfo->tamanioRuta= tamanioRuta;

	size_t tamaniobuffer = sendInfo->tamanioRuta + sendInfo->cantidadDeBytes
			+ sizeof(int) * 3;
	t_pedidoPokedexCliente* pedido = malloc(sizeof(int) * 2);

	operacionBuilder(tamaniobuffer, pedido, LEER_ARCHIVO);

	void* operacionARealizar = malloc(sizeof(int) * 2);

	serializarOperaciones(operacionARealizar, pedido);
	enviar(&socketServer, operacionARealizar, sizeof(int) * 2);
	printf("%i \n", pedido->operacion);
	printf("%i \n", pedido->tamanioBuffer);

	free(operacionARealizar);


	void* bufferSerializado;
	bufferSerializado = malloc(tamaniobuffer);
	serializarMensajeLeerArchivo(bufferSerializado, sendInfo);
	enviar(&socketServer, bufferSerializado, pedido->tamanioBuffer);
	t_MensajeLeerPokedexClient_PokedexServer* deserializadoSeniora;
	deserializadoSeniora = malloc(
			sizeof(t_MensajeLeerPokedexClient_PokedexServer));
	deserializarMensajeLeerArchivo(bufferSerializado, deserializadoSeniora);
	printf("%s \n", deserializadoSeniora->rutaArchivo);
	printf("%s \n", deserializadoSeniora->buffer);
	printf("%d \n", deserializadoSeniora->cantidadDeBytes);
	printf("%d \n", deserializadoSeniora->offset);
	printf("%d \n", deserializadoSeniora->tamanioRuta);
	free(bufferSerializado);
	free(deserializadoSeniora);
	free(pedido);
	log_trace(myLog, "Se quiso leer %s", path);


	recibir(&socketServer,sendInfo->buffer,sendInfo->cantidadDeBytes);


	(void) fi;

	size_t len;


	if (strcmp(path, sendInfo->rutaArchivo) != 0)
		return -ENOENT;
	len = strlen(sendInfo->buffer);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf,sendInfo->buffer + offset ,size);
	} else
		size = 0;


	free(sendInfo);


	return size;


}

static int fuseWrite() {

	return 0;
}

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations fuseOper = { .getattr = fuseGetattr, .readdir =
		fuseReaddir, .open = fuseOpen, .read = fuseRead, .write = fuseWrite, };

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

void openConnection() {
	socket(miSocket, SOCK_STREAM, AF_INET);
	conexion.puerto = 7000;
	conexion.ip = "127.0.0.1";
	socketServer = conectarseA(conexion.ip, conexion.puerto);
}

void enviarMensajeEscribirArchivo() {
	/*
	 conexion.ip="127.0.0.1";
	 conexion.puerto=7000;

	 //int socket = ponerAEscuchar(conexion.ip,conexion.puerto);

	 void* recibirBufferYOperacion = malloc(sizeof(int) * 2);

	 recibir(&socket, recibirBufferYOperacion, sizeof(int)*2);

	 t_pedidoPokedexCliente* operacionARealizar = malloc(sizeof(int)*2);


	 printf("%i \n", operacionARealizar->operacion);
	 printf("%i \n", operacionARealizar->tamanioBuffer);


	 void* bufferARecibir= malloc(8);

	 t_MensajeEscribirArchivoPokedexClient_PokedexServer* escribirArchivo;
	 escribirArchivo = malloc(sizeof(t_MensajeEscribirArchivoPokedexClient_PokedexServer));

	 recibir(&socket, bufferARecibir, operacionARealizar->tamanioBuffer);

	 deserializarMensajeEscribirOModificarArchivo(bufferARecibir,escribirArchivo);

	 printf("%s \n", escribirArchivo->tamanioRuta);
	 printf("%s \n", escribirArchivo->rutaArchivo);
	 printf("%s \n", escribirArchivo->bufferAEscribir);
	 printf("%d \n", escribirArchivo->cantidadDeBytes);
	 printf("%d \n", escribirArchivo->offset);



	 return 0;

	 */
	///prueba SERIALIZADORES LOCALHOST CLIENTE//////
	//conexion.ip="127.0.0.1";
	/*conexion.puerto=7000;
	 int socket;
	 socket = conectarseA(conexion.ip,conexion.puerto);*/
	t_MensajeEscribirArchivoPokedexClient_PokedexServer* pruebaEscribir =
			malloc(1000);
	pruebaEscribir->bufferAEscribir = "Viva el presidente Menem!";
	pruebaEscribir->cantidadDeBytes = strlen(pruebaEscribir->bufferAEscribir);
	pruebaEscribir->offset = 51;
	pruebaEscribir->rutaArchivo = "/home/utnso/titoperez";
	pruebaEscribir->tamanioRuta = string_length(pruebaEscribir->rutaArchivo);
	size_t tamaniobuffer = pruebaEscribir->tamanioRuta
			+ pruebaEscribir->cantidadDeBytes + sizeof(int) * 3;
	t_pedidoPokedexCliente* pedido = malloc(sizeof(int) * 2);
	pedido->operacion = ESCRIBIR_ARCHIVO;
	pedido->tamanioBuffer = tamaniobuffer;
	void* operacionARealizar = malloc(sizeof(int) * 2);
	serializarOperaciones(operacionARealizar, pedido);
	enviar(&socketServer, operacionARealizar, sizeof(int) * 2);
	printf("%i \n", pedido->operacion);
	printf("%i \n", pedido->tamanioBuffer);
	void* bufferSerializado;
	bufferSerializado = malloc(tamaniobuffer);
	serializarMensajeEscribirOModificarArchivo(bufferSerializado,
			pruebaEscribir);
	enviar(&socketServer, bufferSerializado, pedido->tamanioBuffer);
	t_MensajeEscribirArchivoPokedexClient_PokedexServer* deserializadoSeniora;
	deserializadoSeniora = malloc(
			sizeof(t_MensajeEscribirArchivoPokedexClient_PokedexServer));
	deserializarMensajeEscribirOModificarArchivo(bufferSerializado,
			deserializadoSeniora);
	printf("%s \n", deserializadoSeniora->rutaArchivo);
	printf("%s \n", deserializadoSeniora->bufferAEscribir);
	printf("%d \n", deserializadoSeniora->cantidadDeBytes);
	printf("%d \n", deserializadoSeniora->offset);
	printf("%d \n", deserializadoSeniora->tamanioRuta);
	free(bufferSerializado);
	free(deserializadoSeniora);
}

// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS

int main(int argc, char *argv[]) {

	myLog = log_create("Log", "Fuse", 0, 0);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	openConnection();
	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1) {
		// error parsing options //
		perror("Invalid arguments!");
		return EXIT_FAILURE;

	}

	// Si se paso el parametro --welcome-msg
	// el campo welcome_msg deberia tener el
	// valor pasado
	if (runtime_options.welcome_msg != NULL) {
		printf("%s\n", runtime_options.welcome_msg);

	}

	/*
	 conexion.ip="127.0.0.1";
	 conexion.puerto=7000;

	 //int socket = ponerAEscuchar(conexion.ip,conexion.puerto);

	 void* recibirBufferYOperacion = malloc(sizeof(int) * 2);

	 recibir(&socket, recibirBufferYOperacion, sizeof(int)*2);

	 t_pedidoPokedexCliente* operacionARealizar = malloc(sizeof(int)*2);


	 printf("%i \n", operacionARealizar->operacion);
	 printf("%i \n", operacionARealizar->tamanioBuffer);


	 void* bufferARecibir= malloc(8);

	 t_MensajeEscribirArchivoPokedexClient_PokedexServer* escribirArchivo;
	 escribirArchivo = malloc(sizeof(t_MensajeEscribirArchivoPokedexClient_PokedexServer));

	 recibir(&socket, bufferARecibir, operacionARealizar->tamanioBuffer);

	 deserializarMensajeEscribirOModificarArchivo(bufferARecibir,escribirArchivo);

	 printf("%s \n", escribirArchivo->tamanioRuta);
	 printf("%s \n", escribirArchivo->rutaArchivo);
	 printf("%s \n", escribirArchivo->bufferAEscribir);
	 printf("%d \n", escribirArchivo->cantidadDeBytes);
	 printf("%d \n", escribirArchivo->offset);



	 return 0;

	 */

	///prueba SERIALIZADORES LOCALHOST CLIENTE//////
	//conexion.ip="127.0.0.1";
	/*conexion.puerto=7000;
	 int socket;
	 socket = conectarseA(conexion.ip,conexion.puerto);*/

	//enviarMensajeEscribirArchivo();
	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	return fuse_main(args.argc, args.argv, &fuseOper, NULL);

}

char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo) {
	char** arrayDeRuta = string_split(rutaDeArchivoNuevo, "/");
	char* nombreDeArchivo;
	int i = 0;
	while (arrayDeRuta[i + 1] != NULL) {

		i++;
	}

	int tamanio = string_length(arrayDeRuta[i])+1;
	nombreDeArchivo = malloc(tamanio);

	strcpy(nombreDeArchivo, arrayDeRuta[i]);

	return nombreDeArchivo;

}

int stringContains(char* string1, char* string2) {

	if (strstr(string1, string2)) {
		return 1;
	} else
		return 0;

}

////////////////////LOGICA DE LAS FUNCIONES QUE FALTAN PARA YA TENERLO/////////////////////////////////////////
//-Cuando las funciones de fuse esten hechas se pega y se copian estos bloques dentro que incluyen la logica de envios

//Renombrar Archivo
//void renombrarArchivo(char* rutaDeArchivo,char* nuevoNombre);
/*

 tamanioDelBufferAEnviar=strlen(rutaDeArchivo)+strlen(nuevoNombre)+sizeof(int);

 t_mensajeRenombrarArchivoPokedexClient_PokedexServer* infoAEnviar=malloc(tamanioDelBufferAEnviar);

 infoAEnviar->operacion=RENOMBRAR_ARCHIVO;

 infoAEnviar->rutaDeArchivo=rutaDeArchivo;

 infoAEnviar->nuevoNombre=nuevoNombre;

 char* bufferAEnviar=malloc(tamanioDelBufferAEnviar);

 serializarMensajeRenombrarArchivo(bufferAEnviar,infoAEnviar);

 enviar(&socketServer,tamanioDelBufferAEnviar,sizeof(int));

 enviar(&socketServer,bufferAEnviar,tamanioDelBufferAEnviar);

 */

//CrearDirectorio
//void crearDirectorio(char* rutaDirectorioPadre);
/*

 tamanioDelBufferAEnviar=strlen(rutaDirectorioPadre);

 t_mensajeCrearDirectorioPokedexClient_PokedexServer* infoAEnviar=malloc(tamanioDelBufferAEnviar);

 infoAEnviar->operacion=CREAR_DIRECTORIO;

 infoAEnviar->rutaDeDirectorio=rutaDirectorioPadre;

 char* bufferAEnviar=malloc(tamanioDelBufferAEnviar);

 serializarMensajeCrearDirectorio(bufferAEnviar,infoAEnviar);

 enviar(&socketServer,tamanioDelBufferAEnviar,sizeof(int));

 enviar(&socketServer,bufferAEnviar,tamanioDelBufferAEnviar);

 */

//BorrarDirectorio
//void borrarDirectorioVacio(char* rutaDelDirectorioABorrar);
/*

 tamanioDelBufferAEnviar=strlen(rutaDelDirectorioABorrar);

 t_mensajeBorrarDirectorioPokedexClient_PokedexServer* infoAEnviar=malloc(tamanioDelBufferAEnviar);

 infoAEnviar->operacion=BORRAR_DIRECTORIO;

 infoAEnviar->rutaDeDirectorio=rutaDelDirectorioABorrar;

 char* bufferAEnviar=malloc(tamanioDelBufferAEnviar);

 serializarMensajeBorrarDirectorio(bufferAEnviar,infoAEnviar);

 enviar(&socketServer,tamanioDelBufferAEnviar,sizeof(int));

 enviar(&socketServer,bufferAEnviar,tamanioDelBufferAEnviar);

 */

//CrearArchivo
//void crearArchivo(char* rutaArchivoNuevo);
/*

 tamanioDelBufferAEnviar=strlen(rutaArchivoNuevo);

 t_mensajeCrearArchivoPokedexClient_PokedexServer* infoAEnviar=malloc(tamanioDelBufferAEnviar);

 infoAEnviar->operacion=CREAR_ARCHIVO;

 infoAEnviar->rutaDeArchivo=rutaArchivoNuevo;

 char* bufferAEnviar=malloc(tamanioDelBufferAEnviar);

 serializarMensajeCrearArchivo(bufferAEnviar,infoAEnviar);

 enviar(&socketServer,tamanioDelBufferAEnviar,sizeof(int));

 enviar(&socketServer,bufferAEnviar,tamanioDelBufferAEnviar);


 */

//BorrarArchivo
//void borrarArchivos(char* rutaDeArchivo);
/*
 tamanioDelBufferAEnviar=strlen(rutaDeArchivo);

 t_mensajeCrearArchivoPokedexClient_PokedexServer* infoAEnviar=malloc(tamanioDelBufferAEnviar);

 infoAEnviar->operacion=BORRAR_ARCHIVO;

 infoAEnviar->rutaDeArchivo=rutaDeArchivo;

 char* bufferAEnviar=malloc(tamanioDelBufferAEnviar);

 serializarMensajeBorrarArchivo(bufferAEnviar,infoAEnviar);

 enviar(&socketServer,tamanioDelBufferAEnviar,sizeof(int));

 enviar(&socketServer,bufferAEnviar,tamanioDelBufferAEnviar);

 */

