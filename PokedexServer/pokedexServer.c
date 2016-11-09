/*
 * pokedexServer.c
 *
 */

#include "pokedexServer.h"

int main(int argc, char **argv) {

	//char *logFile = NULL;
	//inicializarBloqueCentral();
	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	t_config*configuracion=config_create("/home/utnso/git/tp-2016-2c-SegmentationFault/PokedexServer/ConfigServer");

	conexion.ip=config_get_string_value(configuracion,"IP");
	conexion.puerto=config_get_int_value(configuracion,"PUERTO");
	RUTA_DISCO=config_get_string_value(configuracion,"RUTA_DISCO");

	FILE* discoAbierto = fopen(RUTA_DISCO,"r+");

	void *discoMapeado = mapearArchivoMemoria(discoAbierto);

	mapearEstructura(discoMapeado);

	inicializarSemaforos();

	startServer();


	/*//Parametros
	int i;
	for( i = 0; i < argc; i++){
		if (string_equals_char **argvignore_case(argv[i], "") == 0){
			logFile = argv[i+1];
			printf("Log File: '%s'\n",logFile);
		}
	}

	//Creo el archivo de Log
	//logPokedex = log_create(logFile, "POKEDEXCLIENT", 0, LOG_LEVEL_TRACE);

	startServer();
	 */

/*		char* rutaArchivo = "README.txt";
		char* buffer = malloc(556);

	leerArchivo(rutaArchivo,0,556,buffer);
*/


	return 0;


}

int strcontains(char* cadena1, char* cadena2){


	if(strstr(cadena1,cadena2)!=NULL){


		return 1;

	}

	return 0;

}
////////////////////////////FUNCIONES PROPIAS DEL FILESYSTEM/////////////////////////////////////
void crearArchivo(char* rutaArchivoNuevo){




	//Sacar nombre del archivo de la ruta obtenida
	char* nombreArchivoNuevo = nombreDeArchivoNuevo(rutaArchivoNuevo);




	//Posicion del directorio padre

	int posicionDirectorioPadre = directorioPadrePosicion(rutaArchivoNuevo);

	//Revisar si hay bloques libres para crear archivo

	int bloqueVacio = buscarBloqueVacioEnElBitmap();
	if(bloqueVacio == -1){
		//Avisar al cliente que no hay espacio
		return;

	}
	//Revisar si hay un archivo con el mismo nombre en el directorio padre
	int i;
	for(i=0; i < 2048; i++){
		if(disco->tablaDeArchivos[i].parent_directory == posicionDirectorioPadre && disco->tablaDeArchivos[i].state== REGULAR){
			if(string_equals_ignore_case(disco->tablaDeArchivos[i].fname, nombreArchivoNuevo)){
				printf("No se puede crear archivo. Nombre de archivo existente");
				return;
				//Informar cliente que ya existe un archivo con este nombre
			}

		}
	}
	i++;
	//Buscar un Osada File vacio en la tabla de Archivos
	int osadaFileVacio = 0;
	while(disco->tablaDeArchivos[osadaFileVacio].state != DELETED ){
		osadaFileVacio++;
	}


	////////////////SETEO DE ARCHIVO//////////////////////////


	//Inicializacion de tiempo de creacion

	time_t tiempo;
	struct tm* tm;
	tiempo=time(NULL);
	tm=localtime(&tiempo);

	disco->tablaDeArchivos[osadaFileVacio].lastmod=tm->tm_mday*10000+tm->tm_mon*100+tm->tm_year;


	//Seteo nombre de archivo
	strcpy(&disco->tablaDeArchivos[osadaFileVacio].fname,nombreArchivoNuevo);

	//Seteo estado nuevo del bloque
	disco->tablaDeArchivos[osadaFileVacio].state = REGULAR;

	//Seteo bloque Padre
	disco->tablaDeArchivos[osadaFileVacio].parent_directory = posicionDirectorioPadre;

	//Seteo tamaño inicial del archivo igual a cero(despues se trunca)
	disco->tablaDeArchivos[osadaFileVacio].file_size = 0;

	//Seteo bloque inicial del archivo
	disco->tablaDeArchivos[osadaFileVacio].first_block = ULTIMO_BLOQUE;

	free(nombreArchivoNuevo);
	free(tm);

}



/////////////////// ESCRIBIR O MODIFICAR ARCHIVO ///////////////////

void escribirOModificarArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* bufferAEscribir){

//SE EVALUA SI HAY QUE TRUNCAR EL ARCHIVO

	//Se busca el archivo que es unico en todo el FileSystem
	osada_file archivoAEscribir = buscarArchivoPorRuta(rutaArchivo);

	int tamanioEntrante = offset + cantidadDeBytes;
	if(tamanioEntrante > archivoAEscribir.file_size){

		truncarArchivo(rutaArchivo,tamanioEntrante);

	}
	//Se busca la posicion del archivo a escribir

	int posicionArchivoEscribir = posicionArchivoPorRuta(rutaArchivo);

	//Verifico si alguien mas esta intentando escribir en el archivo
	sem_wait(&semaforos_permisos[posicionArchivoEscribir]);

	//Busco la secuencia de bloques de mi archivo
	t_list* secuenciaDeBloques = crearListaDeSecuencia(archivoAEscribir);


	// Calculo la cantidad de Bloques del archivo en el File System
	double cantidadBloques = ceil((double)archivoAEscribir.file_size / (double)OSADA_BLOCK_SIZE);


	//Busco el comienzo de los bloques de datos
	int i = offset / OSADA_BLOCK_SIZE;

	//Busco el ultimo bloque donde se va a escribir

	int bloqueFinal = (offset + cantidadDeBytes) / OSADA_BLOCK_SIZE;


	//Offset dentro del bloque
	int offsetBloque = offset%OSADA_BLOCK_SIZE;

	//Evaluo el desplazamiento inicial

	int espacioEscrituraInicial = OSADA_BLOCK_SIZE-offsetBloque;
	int desplazamiento = 0;
	int posicionLista =(int)list_get(secuenciaDeBloques,i);

	//Lleno el bloque con fragmentacion interna si es que lo tiene

	if(espacioEscrituraInicial <= cantidadDeBytes ){

		memcpy(disco->bloquesDeDatos[posicionLista] + offsetBloque, bufferAEscribir, espacioEscrituraInicial);
		desplazamiento+=espacioEscrituraInicial;
		i++;
	}

	//Lleno el resto de los bloques
	for(i; i < bloqueFinal ; i++ ){
		posicionLista =(int)list_get(secuenciaDeBloques,i);
		memcpy(disco->bloquesDeDatos[posicionLista],bufferAEscribir+desplazamiento,OSADA_BLOCK_SIZE);
		desplazamiento+=OSADA_BLOCK_SIZE;

	}

	//Se procede a llenar el ultimo bloque

	//Calculo la cantidad de bytes que se leeran del ultimo bloque

	int cantidadBytesUltimoBloque = cantidadDeBytes - desplazamiento;
	posicionLista =(int)list_get(secuenciaDeBloques,i);

	//Copio los bytes del ultimo bloque

	memcpy(disco->bloquesDeDatos[posicionLista],bufferAEscribir+desplazamiento,cantidadBytesUltimoBloque);
	desplazamiento+=cantidadBytesUltimoBloque;

	//Libero el de escritura
	sem_post(&semaforos_permisos[posicionArchivoEscribir]);

	//Libero la lista utilizada
	list_clean(secuenciaDeBloques);
	list_destroy(secuenciaDeBloques);

}

/////////////////// BORRAR ARCHIVOS ///////////////////


void borrarArchivos(char* rutaDeArchivo){

	//Se busca la posicion en la tabla de archivos del archivo a borrar por la ruta dada

	int posicionArchivoBorrar = posicionArchivoPorRuta(rutaDeArchivo);

	//Verifico si alguien mas esta utilizando el archivo
	sem_wait(&semaforos_permisos[posicionArchivoBorrar]);


	//Se busca el archivo que es unico en todo el FileSystem
	osada_file archivoABorrar = buscarArchivoPorRuta(rutaDeArchivo);


	int posicionDelArchivoABorrar = posicionArchivoPorRuta(rutaDeArchivo);

	// Calculo la cantidad de Bloques del archivo en el File System
	double cantidadBloques = calcularBloquesAPedir(archivoABorrar.file_size);


	//Se arma la lista con la secuencia a borrar
	t_list* secuenciaBorrar = crearListaDeSecuencia(archivoABorrar);



	// Se inicia el proceso de borrado de archivo poniendo en 0 el bitmap y el estado de la tabla de Archivos
	// Se pone en 0 la secuencia del archivo en el BitArray demostrando que los bloques de datos ya estan disponible para sobreescribir
	int i;
	int posicionSecuencia;
	int offsetBloqueDeDatos = disco->header.fs_blocks - disco->header.data_blocks;
	for(i = 0; i < cantidadBloques; i++){

		posicionSecuencia = list_get(secuenciaBorrar, i);
		int posicionActual = bitarray_test_bit(disco->bitmap, offsetBloqueDeDatos + posicionSecuencia );
		printf("%i \n", posicionActual);

		bitarray_clean_bit(disco->bitmap, offsetBloqueDeDatos + posicionSecuencia);

		posicionActual = bitarray_test_bit(disco->bitmap, offsetBloqueDeDatos + posicionSecuencia);
		printf("%i \n", posicionActual);


	}
	// Se cambia el estado del archivo a Borrar a DELETED en la tabla de archivos
	disco->tablaDeArchivos[posicionDelArchivoABorrar].state = DELETED;

	//Libero el de permisos
	sem_post(&semaforos_permisos[posicionArchivoBorrar]);

	list_clean(secuenciaBorrar);
	list_destroy(secuenciaBorrar);

}

void crearDirectorio(char* rutaDirectorioPadre){


	//Sacar nombre del archivo de la ruta obtenida
	 char* nombreRuta = nombreDeRutaNueva(rutaDirectorioPadre);

	//Se extrae la posicion del directorio padre
	int posicionDelDirectorioPadre = directorioPadrePosicion(rutaDirectorioPadre);

	//Busco en la tabla de archivos si algun directorio del directorio padre tiene el mismo nombre
	int i;
	for(i=0; i < 2048; i++){
		if(disco->tablaDeArchivos[i].parent_directory == posicionDelDirectorioPadre && disco->tablaDeArchivos[i].state==DIRECTORY){

			if(string_equals_ignore_case(disco->tablaDeArchivos[i].fname, nombreRuta)){
				printf("No se puede crear directorio. Nombre de directorio existente");

			}
		}
	}
	//Busco un lugar vacio en la tabla de archivos
	int j = 0;
	while(disco->tablaDeArchivos[j].state != DELETED ){
		j++;
	}

	//Creo el nuevo directorio

	//Cambio el nombre de la ruta
	strcpy(disco->tablaDeArchivos[j].fname, nombreRuta);

	//Asigno el estado
	disco->tablaDeArchivos[j].state = DIRECTORY;

	//Asigno la posicion del bloque padre
	disco->tablaDeArchivos[j].parent_directory = posicionDelDirectorioPadre;

	//Tamaño del directorio
	disco->tablaDeArchivos[j].file_size = 0;

	//Fecha de creacion
	time_t tiempo;
	struct tm* tm;
	tiempo=time(NULL);
	tm=localtime(&tiempo);
	disco->tablaDeArchivos[j].lastmod=tm->tm_mday*10000+tm->tm_mon*100+tm->tm_year;

	//Bloque inicial no tiene
	disco->tablaDeArchivos[j].first_block = -1;

	free(nombreRuta);
}

/*
void borrarDirectoriosVacios(){
	int i;
	int j=0;
	int cantidadDeDirectorios=contarCantidadDeDirectorios();
	int* arrayDirectorios=malloc(cantidadDeDirectorios*sizeof(int));
	for(i=0;i<1024;i++){
		if(disco->tablaDeArchivos[i].state==DIRECTORY){
			arrayDirectorios[j]=i;
		}
	}
	eliminarDirectoriosVacios(arrayDirectorios);
}
 */


void borrarDirectorioVacio(char* rutaDelDirectorioABorrar){

	//busco la posicion del directorio en la tabla de archivos, el cual es unico.
	int posicionDirectorio = posicionArchivoPorRuta(rutaDelDirectorioABorrar);


	//Recorro las 2048 posiciones de la tabla de archivos, en caso de que ninguna lo tenga como directorio padre se lo borra
	int i;
	for(i = 0; i < 2048; i++){
		if(disco->tablaDeArchivos[i].parent_directory == posicionDirectorio){

			printf("No se puede eliminar directorio ya que contiene archivos dentro \n");
			return;

		}
	}

	//Se procede a marcar como eliminado el directorio

	disco->tablaDeArchivos[posicionDirectorio].state = DELETED;

}



/////////////////// RENOMBRAR ARCHIVOS ///////////////////

void renombrarArchivo(char* rutaDeArchivo, char* nuevoNombre){

	//Se busca la posicion en la tabla de archivos del archivo a borrar por la ruta dada

	int posicionArchivoRenombrar= posicionArchivoPorRuta(rutaDeArchivo);

	//Se verifica si es posible renombrar utilizando semaforos.

	//Verifico si alguien mas esta intentando escribir en el archivo
	sem_wait(&semaforos_permisos[posicionArchivoRenombrar]);


	//Verificar que el nuevo nombre no tenga mas de 17 caracteres
	int resultadoCantidadDeCaracteres = string_length(nuevoNombre);
	if(resultadoCantidadDeCaracteres > 17){
		return;
	}

	//Se busca el archivo que es unico en todo el FileSystem
	osada_file archivoARenombrar = buscarArchivoPorRuta(rutaDeArchivo);

	//Se busca si un archivo en el directorio contiene el mismo nombre
	int resultado = revisarMismoNombre(archivoARenombrar, nuevoNombre);

	//Si no se encuentra un archivo con el mismo nombre en el directorio padre se procede a cambiar el nombre
	if(resultado){

		memcpy(disco->tablaDeArchivos[posicionArchivoRenombrar].fname, nuevoNombre, resultadoCantidadDeCaracteres );

	}

	//Libero los semaforos utilizados


	//Libero el de lectura
	sem_post(&semaforos_permisos[posicionArchivoRenombrar]);




}

//RENOMBRAR O MOVER ARCHIVO

void moverArchivo(char* rutaOrigen, char* rutaDestino){

	//Se busca la posicion en la tabla de archivos del archivo a mover por la ruta de origen dada

	int posicionArchivoRenombrar = posicionArchivoPorRuta(rutaOrigen);

	//Verifico si alguien mas esta intentando escribir/modificar/borrar en el archivo
	sem_wait(&semaforos_permisos[posicionArchivoRenombrar]);

	//Se busca la posicion del directorio padre de la ruta de origen
	int posicionDirectorioPadre = directorioPadrePosicion(rutaDestino);


	//Se obtiene el nombre que le pondremos al nuevo archivo
	char* nombreArchivoNuevo = nombreDeArchivoNuevo(rutaDestino);

	//Se recorre la tabla de archivos para verificar si alguno tiene el mismo nombre en el directorio padre
	int i;
	for( i = 0; i < 2048; i++){

		if(string_equals_ignore_case(nombreArchivoNuevo, disco->tablaDeArchivos[i].fname) && disco->tablaDeArchivos[i].parent_directory == posicionDirectorioPadre)
		{

			perror("Ya existe un archivo con el mismo nombre en el directorio padre");
			return;
		}
	}

	//Se procede a cambiar el directorio padre del archivo a mover, para eso buscamos su posicion en la tabla de archivos

	disco->tablaDeArchivos[posicionArchivoRenombrar].parent_directory = posicionDirectorioPadre;

	strcpy(disco->tablaDeArchivos[posicionArchivoRenombrar].fname, nombreArchivoNuevo);

	//Libero los semaforos utilizados

	//Libero el semaforo de permisos de escritura/borrado/mover/truncado.

	sem_post(&semaforos_permisos[posicionArchivoRenombrar]);


	free(nombreArchivoNuevo);


}
/////////LISTAR ARCHIVOS////////

void listarArchivos(char* rutaDirectorio, int* socketEnvio){

	//Saco la posicion del directorio
	int posicionDirectorio = posicionArchivoPorRuta(rutaDirectorio);

	//Inicializo las variables
	int posicionTablaDeArchivos;
	int cantidadDeArchivos = 0;

	//Reviso cuantos archivos se encuentran en el directorio
	printf("Dentro del directorio %s se encuentra el archivo: \n", disco->tablaDeArchivos[posicionDirectorio].fname);
	for(posicionTablaDeArchivos = 0; posicionTablaDeArchivos < 2048; posicionTablaDeArchivos++){

		if(disco->tablaDeArchivos[posicionTablaDeArchivos].parent_directory == posicionDirectorio && disco->tablaDeArchivos[posicionTablaDeArchivos].state!= DELETED){

			printf("%s \n", disco->tablaDeArchivos[posicionTablaDeArchivos].fname);

			cantidadDeArchivos++;

		}
	}
	//Tamaño de la estructura
	int tamanio = 17*cantidadDeArchivos;

	//Asigno 18 bytes por la cantidad de archivos encontrados ya que cada uno sera un unsigned char[17]
	char *listado = malloc(tamanio);

	//Asigno el nombre de los archivos y directorios encontrados al buffer a enviar y seteo un offset en 0 para avanzar en memcpy

	int offset=0;


	for(posicionTablaDeArchivos = 0; posicionTablaDeArchivos < 2048; posicionTablaDeArchivos++){

		if(disco->tablaDeArchivos[posicionTablaDeArchivos].parent_directory == posicionDirectorio && disco->tablaDeArchivos[posicionTablaDeArchivos].state!= DELETED){

			//Se copian en memoria los nombres de los directorios y archivos encontrados
			//en la direccion de la tabla de archivos
			memcpy(listado + offset, &(disco->tablaDeArchivos[posicionTablaDeArchivos].fname), 17);
			offset += 17;

		}



	}

	memcpy(&tamanio,&offset,sizeof(int));

	enviar(socketEnvio,&tamanio,sizeof(int));

	enviar(socketEnvio,listado,tamanio);

	free(listado);



}


///////////////////COPIAR ARCHIVO/////////////////////////////

void copiarArchivo(char* rutaArchivo, char* rutaCopia){

	//Busco el archivo a copiar
	osada_file copiarArchivo = buscarArchivoPorRuta(rutaArchivo);

	/*Armo la ruta del nuevo directorio
	char* rutaCopiaDeArchivo=string_new();
	string_append_with_format(rutaCopiaDeArchivo, copiarArchivo.fname, "%s/");
	 */

	//Creo el archivo en el nuevo directorio

	crearArchivo(rutaCopia);

	//Trunco el tamaño del archivo nuevo para que sea igual al archivo de origen

	truncarArchivo(rutaCopia, copiarArchivo.file_size);

	//Leo el archivo entero y lo guardo en el buffer

	char* buffer = malloc(copiarArchivo.file_size);

	leerArchivo(rutaArchivo,0,copiarArchivo.file_size,buffer);

	//Escribo el archivo nuevo

	escribirOModificarArchivo(rutaCopia,0,copiarArchivo.file_size,buffer);



}

void truncarArchivo(char* rutaArchivo, int cantidadDeBytes){

	//Se busca la posicion original en la tabla de archivos
	int posicionArchivoTruncar = posicionArchivoPorRuta(rutaArchivo);


	//Verifico si alguien mas esta intentando escribir/modificar/borrar en el archivo
	sem_wait(&semaforos_permisos[posicionArchivoTruncar]);


	//Busco el archivo que es unico en el filesystem
	osada_file archivoATruncar = buscarArchivoPorRuta(rutaArchivo);



	int bloquesActuales = calcularBloquesAPedir(archivoATruncar.file_size);


	//Bloques necesarios que se necesitan para escribir
	int bloquesNecesarios = calcularBloquesAPedir(cantidadDeBytes);


	int bloquesAgregar = bloquesNecesarios - bloquesActuales;


	//Offset donde comienzan los bloques de datos
	int offset = disco->header.fs_blocks - disco->header.data_blocks;


	if(bloquesAgregar > 0){

		//Verifico que existan bloques de datos disponibles
		int bloquesVacios = cantidadDeBloquesVacios();
		if(bloquesVacios < bloquesAgregar){
			printf("No se encuentran bloques vacios");
			//Comunicarse con el cliente y avisarle que no hay espacio

			return;
		}

		/////////////ARCHIVO NUEVO/////////////////
		//Si el archivo es nuevo se debe modificar la tabla de asignaciones con el primer bloque

		if(archivoATruncar.file_size == 0){

			//El bitmap siempre nos va a devolver un bloque de datos en su posicion de los datos,
			//Si se le resta el total de bloques del file system se obtendra la posicion de la tabla de asignacion

			int lugarBloqueVacio = buscarBloqueVacioEnElBitmap();
			int posicionBloqueDisponible = lugarBloqueVacio - offset;
			disco->tablaDeArchivos[posicionArchivoTruncar].first_block = posicionBloqueDisponible;
			disco->tablaDeAsignaciones[posicionBloqueDisponible] = ULTIMO_BLOQUE;
			int nuevoBloque;
			int j;
			for(j=1;j<bloquesAgregar;j++){

				lugarBloqueVacio = buscarBloqueVacioEnElBitmap();
				nuevoBloque = lugarBloqueVacio-offset;
				disco->tablaDeAsignaciones[posicionBloqueDisponible] = nuevoBloque;
				posicionBloqueDisponible = nuevoBloque;
				disco->tablaDeAsignaciones[nuevoBloque] = ULTIMO_BLOQUE;


			}

		}


		//Si el archivo ya esta en uso se le agregan bloques desde su ultima posicion


		if(archivoATruncar.file_size > 0){


			double ultimoBloqueArchivo = calcularBloquesAPedir(archivoATruncar.file_size);
			int ultimoBloque = ultimaPosicionBloqueDeDatos(archivoATruncar);
			int m;
			int bloqueNuevo;
			int lugarDeBloqueVacio;

			for(m=0;m<bloquesNecesarios;m++){

				lugarDeBloqueVacio = buscarBloqueVacioEnElBitmap();
				bloqueNuevo = lugarDeBloqueVacio - offset;
				disco->tablaDeAsignaciones[ultimoBloque] = bloqueNuevo;
				ultimoBloque = bloqueNuevo;
				disco->tablaDeAsignaciones[bloqueNuevo] = ULTIMO_BLOQUE;


			}

		}

	}
	//SI EL TAMAÑO DE BYTES RECIBIDOS ES MENOR QUE EL TAMAÑO DEL ARCHIVO SE PROCEDE A QUITARLE BLOQUES
	if(cantidadDeBytes < archivoATruncar.file_size){

		//Calculo los bloques a quitar

		int cantidadBloquesAQuitar = abs(bloquesAgregar);

		int p=0;

		//Elimino la cantidad de bloques necesarios

		while(p < cantidadBloquesAQuitar){



			eliminarUltimoBloqueDeArchivo(posicionArchivoTruncar);
			p++;

		}


	}
	if(cantidadDeBytes == 0){


		borrarBloqueDeDatosEnElBitmap(disco->tablaDeArchivos[posicionArchivoTruncar].first_block);
		disco->tablaDeArchivos[posicionArchivoTruncar].first_block = ULTIMO_BLOQUE;






	}
	//SE LE ASIGNA EL TAMAÑO NUEVO AL ARCHIVO

	disco->tablaDeArchivos[posicionArchivoTruncar].file_size=cantidadDeBytes;



	//Libero el semaforo de permisos de escritura/borrado/mover.
	sem_post(&semaforos_permisos[posicionArchivoTruncar]);





}


//////////////////FUNCION LEER AUXILIAR//////////////////////////////
void leerArchivo(char* rutaArchivo,int offset,int cantidadALeer,char* buffer){

	//Busco la posicion del archivo en la tabla
	int posicionLeerArchivo = posicionArchivoPorRuta(rutaArchivo);

	//Verifico que nadie este escribiendo/modificando/borrando
	sem_wait(&semaforos_permisos[posicionLeerArchivo]);


	//Busco el archivo en mi tabla de Archivos
	osada_file archivoALeer=disco->tablaDeArchivos[posicionLeerArchivo];


	int bufferSobrante = cantidadALeer - archivoALeer.file_size;

	int cantidadDeBytes;

	if(archivoALeer.file_size < cantidadALeer){

		cantidadDeBytes = archivoALeer.file_size;


	}else{


		cantidadDeBytes = cantidadALeer;


	}
	//Armo mi secuencia de bloques usando la tabla de asginaciones


	t_list* bloquesLectura = crearListaDeSecuencia(archivoALeer);

	// Calculo la cantidad de Bloques del archivo en el File System
	double cantidadBloques = ceil((double)archivoALeer.file_size / (double)OSADA_BLOCK_SIZE);

	//Busco el bloque de inicio y lleno si es que tiene espacio libre
	int i= offset / OSADA_BLOCK_SIZE;

	//Busco el bloque donde finaliza la lectura
	int bloqueFinal = (offset + cantidadDeBytes) / OSADA_BLOCK_SIZE;


	//Busco el desplazamiento inicial
	int offsetBloque = offset % OSADA_BLOCK_SIZE;
	int desplazamiento = 0;

	//Calculo la cantidad inicial de escritura del primer bloque
	int espacioLecturaInicial = OSADA_BLOCK_SIZE - offsetBloque;

	if(espacioLecturaInicial <= cantidadDeBytes ){

		memcpy(buffer, disco->bloquesDeDatos[(int)list_get(bloquesLectura,i)] + offsetBloque, espacioLecturaInicial);
		desplazamiento+=espacioLecturaInicial;
		i++;
	}

	//Leo el resto de los bloques de manera completa

	for(i; i < bloqueFinal ; i++ ){


		int aux = list_get(bloquesLectura,i);
		memcpy(buffer+desplazamiento,disco->bloquesDeDatos[aux],OSADA_BLOCK_SIZE);

		desplazamiento+=OSADA_BLOCK_SIZE;

	}

	//Calculo la cantidad de bytes que se leeran del ultimo bloque

	int cantidadBytesUltimoBloque = cantidadDeBytes - desplazamiento;


	//Copio los bytes del ultimo bloque


	memcpy(buffer+desplazamiento,disco->bloquesDeDatos[(int)list_get(bloquesLectura,i)],cantidadBytesUltimoBloque);
	desplazamiento+=cantidadBytesUltimoBloque;


	//Se llena de basura el resto del buffer


	memset(buffer+desplazamiento,'\0',bufferSobrante);


	printf("%s\n",buffer);
	//memcpy(buffer,parteDelArchivoALeer+offset,cantidadDeBytes);
	//Enviar al cliente la seccion del archivo que pidio
<<<<<<< HEAD
	free(secuenciaDeBloqueALeer);
=======

>>>>>>> 2db423fcbc210001e44f82c35ca60a5a26d13ebd

	//Libero el semaforo de Lectura
	sem_post(&semaforos_permisos[posicionLeerArchivo]);

	list_clean(bloquesLectura);
	list_destroy(bloquesLectura);

}

void atributosArchivo(char* rutaArchivo, int* socket){

	t_MensajeAtributosArchivoPokedexServer_PokedexClient* atributosArchivo = malloc(sizeof(t_MensajeAtributosArchivoPokedexServer_PokedexClient));

	int posicionArchivoAtributo = posicionArchivoPorRuta(rutaArchivo);


	if(posicionArchivoAtributo == -1 ){

		atributosArchivo->estado = -1;
		atributosArchivo->tamanio = -1;

	}



	else if(posicionArchivoAtributo == ROOT_DIRECTORY){

		atributosArchivo->estado = DIRECTORY;
		atributosArchivo->tamanio = 0;


	}
	else{


		atributosArchivo->estado = disco->tablaDeArchivos[posicionArchivoAtributo].state;
		atributosArchivo->tamanio = disco->tablaDeArchivos[posicionArchivoAtributo].file_size;

	}

	void* buffer = malloc(sizeof(int) * 2);

	serializarAtributos(buffer, atributosArchivo);

	enviar(socket,buffer, sizeof(int) * 2);

	free(buffer);
	free(atributosArchivo);

}




/////////////////////////////FUNCIONES SECUNDARIAS//////////////////////////////////////
void seteoInicialTablaDeAsignaciones(int* tablaDeAsignaciones){
	int i;
	for(i=0;i>=2048;i++){
		tablaDeAsignaciones[i]=-1;
	}
}

void* mapearArchivoMemoria(FILE* archivo){
	int tamanio = calcularTamanioDeArchivo(archivo);
	int descriptorArchivo;
	descriptorArchivo = fileno(archivo);
	lseek(descriptorArchivo, 0, SEEK_SET);
	disco = malloc(tamanio);
	void* archivoMapeado=malloc(tamanio);
	archivoMapeado = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, descriptorArchivo, 0);
	return archivoMapeado;


}


int calcularTamanioDeArchivo(FILE* archivoAMapear){
	fseek(archivoAMapear, 0, SEEK_END);
	int tamanio=ftell(archivoAMapear);
	return tamanio;
}


void inicializarBloqueCentral(){
	int cantidadDeBloquesTotal = tamanioDisco / OSADA_BLOCK_SIZE;
	disco->header.version=1;
	disco->header.fs_blocks=ceil(tamanioDisco/OSADA_BLOCK_SIZE);
	disco->header.bitmap_blocks=ceil(disco->header.fs_blocks/8/OSADA_BLOCK_SIZE);
	disco->header.allocations_table_offset=disco->header.bitmap_blocks+1025;


	int cantidadDeBloquesTablaDeAsignaciones=ceil((cantidadDeBloquesTotal-1-disco->header.bitmap_blocks-1024)*4/OSADA_BLOCK_SIZE);
	disco->header.data_blocks=cantidadDeBloquesTotal-cantidadDeBloquesTablaDeAsignaciones-disco->header.bitmap_blocks-1-1024;
	int i;
	int bloquesAdministrativos=1+1024+disco->header.bitmap_blocks+cantidadDeBloquesTablaDeAsignaciones;
	char* bitmap = malloc(sizeof(cantidadDeBloquesTotal));
	disco->bitmap = bitarray_create(bitmap, cantidadDeBloquesTotal);
	for(i=0;i<=bloquesAdministrativos;i++){
		bitarray_set_bit(disco->bitmap,i);
	}
	for(i=bloquesAdministrativos+1;i<=10000;i++){
		bitarray_clean_bit(disco->bitmap,i);
	}

	seteoInicialTablaDeAsignaciones(disco->tablaDeAsignaciones);
	int j;
	for(j=0;j<2048;j++){
		disco->tablaDeArchivos[j].file_size=-1;
	}
}

int buscarBloqueVacioEnElBitmap(){
	int i=0;



	while(bitarray_test_bit(disco->bitmap,i)!=0){
		i++;


	}
	if(i==disco->header.bitmap_blocks){
		return -1;

	}
	bitarray_set_bit(disco->bitmap,i);
	return i;


}

void completarTablaDeAsignaciones(int* tablaDeAsignaciones,int cantidadDeBloquesArchivo,int primerBloque){
	int i;
	int primerBloqueVacio=buscarBloqueVacioEnElBitmap();
	tablaDeAsignaciones[primerBloque]=primerBloqueVacio;
	for(i=1;i>cantidadDeBloquesArchivo;i++){
		if(i==cantidadDeBloquesArchivo){
			tablaDeAsignaciones[primerBloqueVacio]=-2;
		}
		tablaDeAsignaciones[primerBloqueVacio]=buscarBloqueVacioEnElBitmap();
		primerBloqueVacio=buscarBloqueVacioEnElBitmap();

	}

}

void copiarArchivoNuevoEnMemoria(void* fsMapeado,int* tablaDeAsignaciones,int primerBloque,int cantidadDeBloquesArchivo){
	int offset=sizeof(osada_header);
	int i;
	for(i=0;i<cantidadDeBloquesArchivo;i++){
		offset=+tablaDeAsignaciones[primerBloque]*OSADA_BLOCK_SIZE;
		//memcpy(fsMapeado+offset);
	}
}


osada_file buscarArchivoPorRuta(char* rutaAbsolutaArchivo){

	//Separo la ruta recibida en un array de strings.


	char** arrayDeRuta= string_split(rutaAbsolutaArchivo, "/");


	//Se busca el numero de directorio del primer directorio que tiene como padre al directorio raiz

	int j=0;

	while(!(string_equals_ignore_case(arrayDeRuta[0],disco->tablaDeArchivos[j].fname) && disco->tablaDeArchivos[j].parent_directory == ROOT_DIRECTORY  && disco->tablaDeArchivos[j].state!=DELETED) ){
		j++;
	}

	//Se guarda el directorio anterior, el cual sera el padre del proximo elemento en el array de ruta.
	int directorioAnterior;

	directorioAnterior=j;

	//Se analiza el array recursivamente a partir del segundo elemento para encontrar el directorio padre de cada uno, el cual sera unico.

	// La variable k recorre las posiciones de la tabla de archivos y la variable "i" recorre el array de rutas.
	int k;
	int i = 1;

	while(arrayDeRuta[i]!=NULL){

		//Seteo en 0 para recorrer la tabla de archivos desde un principio
		k=0;
		while(!(string_equals_ignore_case(arrayDeRuta[i],disco->tablaDeArchivos[k].fname) && (disco->tablaDeArchivos[k].parent_directory==directorioAnterior && disco->tablaDeArchivos[j].state!=DELETED)))
		{

			k++;

		}

		//El directorio encontrado sera el directorio padre del siguiente en la ruta.
		directorioAnterior=k;
		i++;
	}

	int m=0;
	while(arrayDeRuta[m]!=NULL){
		free(arrayDeRuta[m]);
		m++;
	}
	free(arrayDeRuta);

	return disco->tablaDeArchivos[k];

}



int* buscarSecuenciaBloqueDeDatos(osada_file archivo){
	int* secuencia;
	double tamanioAReservar = ceil((double)archivo.file_size / (double)OSADA_BLOCK_SIZE);
	secuencia = malloc(tamanioAReservar * sizeof(int));


	int i = archivo.first_block;
	secuencia[0]=archivo.first_block;

	int j=1;
	while(disco->tablaDeAsignaciones[i]!= -1){
		secuencia[j] = disco->tablaDeAsignaciones[i];
		i=disco->tablaDeAsignaciones[i];
		//printf("secuencia[%i]: %i \n",j, secuencia[j]);
		//printf("Siguiente posicion en la Tabla de asignaciones: %i \n", disco->tablaDeAsignaciones[i]);
		j++;
	}

	secuencia[j] = ULTIMO_BLOQUE;
	//printf("secuencia[%i]: %i \n",j, secuencia[j]);
	return secuencia;

}

int llenarEspacioLibreUltimoBloque(int* secuencia,char* loQueVoyAEscribir){

	int i = 0;
	while(secuencia[i]!= NULL){
		i++;
	}
	int ultimoBloqueDeArchivo = i;
	char* ultimoBloque=malloc(OSADA_BLOCK_SIZE);
	memcpy(ultimoBloque,disco->bloquesDeDatos+i*OSADA_BLOCK_SIZE,64);
	int j=0;
	while(ultimoBloque[j]!='\0'){
		j++;
	}
	int k=j;
	int z=0;
	while(ultimoBloque[k]!=NULL){
		ultimoBloque[k]=loQueVoyAEscribir[z];
		k++;
		z++;
	}
	memcpy(disco->bloquesDeDatos+i*OSADA_BLOCK_SIZE,ultimoBloque,64);
	return z;
}

double calcularBloquesAPedir(int bytesRestantes){

	double bytes = (double)bytesRestantes;

	double bloquesAPedir = ceil(bytes / (double)OSADA_BLOCK_SIZE);

	return bloquesAPedir;

}

int calcularPosicionDeEscrituraUltimoBloque(int cantidadRestanteAEscribir){

	int posicionUltimoBloque = cantidadRestanteAEscribir % OSADA_BLOCK_SIZE;

	return posicionUltimoBloque;

}

int calcularUltimoBloque(int* secuencia){

	int i = 0;

	while (secuencia[i+1]!= ULTIMO_BLOQUE){

		i++;
	}

	return secuencia[i];

}
int revisarMismoNombre(osada_file archivoARenombrar, char* nuevoNombre){

	int i;
	for(i = 0; i < 2047; i++){

		if(disco->tablaDeArchivos[i].parent_directory == archivoARenombrar.parent_directory){
			if(string_equals_ignore_case(disco->tablaDeArchivos[i].fname,nuevoNombre)){
				printf("Error: nombre existente en este directorio");
				return -1;
			}
		}

	}

	return 1;

}
int posicionArchivoPorRuta(char* rutaAbsolutaArchivo){

	//Analizo primero si me pide analizar el directorio padre

	if(string_equals_ignore_case(rutaAbsolutaArchivo, "/")){

		return ROOT_DIRECTORY;


	}




	//Separo la ruta recibida en un array de strings.

	char** arrayDeRuta= string_split(rutaAbsolutaArchivo, "/");


	//Se busca el numero de directorio del primer directorio que tiene como padre al directorio raiz

	int j=0;




	while(!(string_equals_ignore_case(arrayDeRuta[0],disco->tablaDeArchivos[j].fname)
			&& disco->tablaDeArchivos[j].parent_directory == ROOT_DIRECTORY
			&& disco->tablaDeArchivos[j].state!=DELETED) )
	{
		j++;

		if(j == 2048){
			return -1;
		}
	}

	//Se guarda el directorio anterior, el cual sera el padre del proximo elemento en el array de ruta.
	int directorioAnterior=0;

	directorioAnterior=j;

	//Se analiza el array recursivamente a partir del segundo elemento para encontrar el directorio padre de cada uno, el cual sera unico.

	// La variable k recorre las posiciones de la tabla de archivos y la variable "i" recorre el array de rutas.
	int k;
	int i = 1;
	if(arrayDeRuta[i]==NULL){

		k=directorioAnterior;

	}
	while(arrayDeRuta[i]!=NULL){

		//Seteo en 0 para recorrer la tabla de archivos desde un principio
		k=0;
		while(!	(string_equals_ignore_case(arrayDeRuta[i],disco->tablaDeArchivos[k].fname)
				&& disco->tablaDeArchivos[k].parent_directory==directorioAnterior
				&& disco->tablaDeArchivos[j].state!=DELETED)	)
		{

			k++;
			if(k==2048){
				return -1;
			}

		}

		//El directorio encontrado sera el directorio padre del siguiente en la ruta.
		directorioAnterior=k;
		i++;
	}

	int m=0;
	while(arrayDeRuta[m]!=NULL){
		free(arrayDeRuta[m]);
		m++;
	}

	free(arrayDeRuta);

	//Se devuelve la posicion en la tabla de archivos
	return k;
}

int directorioPadrePosicion(char* rutaAbsolutaArchivo){


	//Analizo primero si me pide analizar el directorio padre

	if(string_equals_ignore_case(rutaAbsolutaArchivo, "/")){

		return ROOT_DIRECTORY;


	}

	//Separo la ruta recibida en un array de strings.

	char** arrayDeRuta = string_split(rutaAbsolutaArchivo, "/");


	//Se busca el numero de directorio del primer directorio que tiene como padre al directorio raiz

	int j=0;

	while(!(string_equals_ignore_case(arrayDeRuta[0],disco->tablaDeArchivos[j].fname) && disco->tablaDeArchivos[j].parent_directory == ROOT_DIRECTORY && disco->tablaDeArchivos[j].state != DELETED) ){
		j++;

		if(j==2048){
			return -1;
		}
	}

	//Se guarda el directorio anterior, el cual sera el padre del proximo elemento en el array de ruta.
	int directorioAnterior;

	directorioAnterior=j;

	//Se analiza el array recursivamente a partir del segundo elemento para encontrar el directorio padre de cada uno, el cual sera unico.

	// La variable k recorre las posiciones de la tabla de archivos y la variable "i" recorre el array de rutas.
	int k = directorioAnterior;
	int i = 1;

	while(arrayDeRuta[i+1]!=NULL){

		//Seteo en 0 para recorrer la tabla de archivos desde un principio
		k=0;
		while(!(string_equals_ignore_case(arrayDeRuta[i],disco->tablaDeArchivos[k].fname) && (disco->tablaDeArchivos[k].parent_directory==directorioAnterior) && (disco->tablaDeArchivos[k].state != DELETED)))
		{

			k++;
			if(k==2048){
				return -1;
			}
		}

		//El directorio encontrado sera el directorio padre del siguiente en la ruta.
		directorioAnterior=k;
		i++;
	}

	//Se devuelve la posicion en la tabla de archivos
	return k;
}




int eliminarDirectoriosVacios(int* arrayDirectorios){
	int k=0;
	while(arrayDirectorios[k]!=NULL){
		int z;
		for(z=0;z<1024;z++){
			if(arrayDirectorios[k]==disco->tablaDeArchivos[z].parent_directory){
				z=1024;
			}else{
				disco->tablaDeArchivos[z].state=DELETED;
			}
		}
	}
	return 0;
}

int contarCantidadDeDirectorios(){
	int i;
	int k;
	int acum=0;
	int flag=0;
	for(i=0;i<2048;i++){
		for(k=0;k<2048;k++){
			if(disco->tablaDeArchivos[k].parent_directory==i){
				flag=1;
			}

		}
		if(flag==1){
			acum++;
		}
	}
	return acum;
}



void escucharOperaciones(int* socketCliente){

	//Reservo espacio para la operacion a realizar y la cantidad de bytes necesarios para el buffer

	void* bufferOperacionTamanio=malloc(sizeof(int)*2);

	//Reservo Espacio para la estructura a utilizar

	t_pedidoPokedexCliente* operacionYtamanio=malloc(sizeof(int)*2);

	//Recibo los datos del cliente

	recibir(socketCliente,bufferOperacionTamanio,sizeof(int)*2);
	deserializarOperaciones(bufferOperacionTamanio,operacionYtamanio);
	free(bufferOperacionTamanio);
	//Reservo memoria para recibir el buffer del cliente

	void* bufferRecibido = malloc(operacionYtamanio->tamanioBuffer);
	recibir(socketCliente,bufferRecibido,operacionYtamanio->tamanioBuffer);

	printf("%i \n",operacionYtamanio->operacion);

	switch(operacionYtamanio->operacion) {

	case LEER_ARCHIVO:{

		//Reservo espacio para la estructura con la que se va a trabajar
		t_MensajeLeerPokedexClient_PokedexServer* lecturaNueva=malloc(sizeof(t_MensajeLeerPokedexClient_PokedexServer));
		deserializarMensajeLeerArchivo(bufferRecibido,lecturaNueva);



		printf("La cantidad de bytes a leer es: %i\n",lecturaNueva->cantidadDeBytes);
		printf("El offset donde comienza el archivo es: %i\n",lecturaNueva->offset);
		printf("El tamanio de la ruta a escribir es: %i \n",lecturaNueva->tamanioRuta);
		printf("La ruta a leer es: %s\n",lecturaNueva->rutaArchivo);

		leerArchivo(lecturaNueva->rutaArchivo,lecturaNueva->offset,lecturaNueva->cantidadDeBytes,lecturaNueva->buffer);

		enviar(socketCliente,lecturaNueva->buffer,lecturaNueva->cantidadDeBytes);


		//Libero las estructuras utilizadas
		free(lecturaNueva->rutaArchivo);
		free(lecturaNueva->buffer);
		free(lecturaNueva);
		free(operacionYtamanio);
		free(bufferRecibido);
		break;
	}

	case CREAR_ARCHIVO:{

		//Reservo espacio para la estructura a trabajar

		t_MensajeCrearArchivoPokedexClient_PokedexServer* archivoNuevo=malloc(sizeof(t_MensajeCrearArchivoPokedexClient_PokedexServer));

		//Deserializo el mensaje con la informacion enviada en la estructura recientemente creada
		deserializarMensajeCrearArchivo(bufferRecibido,archivoNuevo);

		//Procedemos a crear el archivo
		crearArchivo(archivoNuevo->rutaDeArchivoACrear);

		//Libero las estructuras utilizadas
		free(archivoNuevo->rutaDeArchivoACrear);
		free(archivoNuevo);
		free(operacionYtamanio);
		free(bufferRecibido);
		break;
	}

	case ESCRIBIR_ARCHIVO :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeEscribirArchivoPokedexClient_PokedexServer* escrituraNueva=malloc(sizeof(t_MensajeEscribirArchivoPokedexClient_PokedexServer));

		//Deserializo el buffer recibido

		deserializarMensajeEscribirOModificarArchivo(bufferRecibido,escrituraNueva);

		//Procedemos a escribir el archivo
		escribirOModificarArchivo(escrituraNueva->rutaArchivo,escrituraNueva->offset,escrituraNueva->cantidadDeBytes, escrituraNueva->bufferAEscribir);

		//Libero las estructuras utilizadas
		free(escrituraNueva->bufferAEscribir);
		free(escrituraNueva->rutaArchivo);
		free(escrituraNueva);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;
	}

	case BORRAR_ARCHIVO :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeBorrarArchivoPokedexClient_PokedexServer* borradoNuevo=malloc(sizeof(t_MensajeBorrarArchivoPokedexClient_PokedexServer));

		//Deserializo el buffer recibido

		deserializarMensajeBorrarArchivo(bufferRecibido,borradoNuevo);

		//Se procede a borrar el archivo

		borrarArchivos(borradoNuevo->rutaArchivoABorrar);

		//Libero las estructuras utilizadas
		free(borradoNuevo->rutaArchivoABorrar);
		free(borradoNuevo);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;
	}

	case CREAR_DIRECTORIO :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeCrearDirectorioPokedexClient_PokedexServer* directorioNuevo=malloc(sizeof(t_MensajeCrearDirectorioPokedexClient_PokedexServer));

		//Deserializo el buffer recibido

		deserializarMensajeCrearDirectorio(bufferRecibido,directorioNuevo);

		//Se procede a crear el directorio nuevo
		crearDirectorio(directorioNuevo->rutaDirectorioPadre);


		//Libero las estructuras utilizadas
		free(directorioNuevo->rutaDirectorioPadre);
		free(directorioNuevo);
		free(operacionYtamanio);
		free(bufferRecibido);
		break;
	}

	case BORRAR_DIRECTORIO :{

		//Reservo espacio para la estructura a utilizar


		t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* directorioABorrar=malloc(sizeof(t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer));

		//Deserializo el mensaje recibido

		deserializarMensajeBorrarDirectorio(bufferRecibido,directorioABorrar);

		//Se procede a borrar el directorio vacio

		borrarDirectorioVacio(directorioABorrar->rutaDirectorioABorrar);
		//Libero las estructuras utilizadas
		free(directorioABorrar->rutaDirectorioABorrar);
		free(directorioABorrar);
		free(operacionYtamanio);
		free(bufferRecibido);
		break;
	}

	case RENOMBRAR_ARCHIVO :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeRenombrarArchivoPokedexClient_PokedexServer* archivoARenombrar=malloc(sizeof(t_MensajeRenombrarArchivoPokedexClient_PokedexServer));

		//Deserializo el mensaje recibido
		deserializarMensajeRenombrarArchivo(bufferRecibido,archivoARenombrar);

		//Se procede a renombrar archivo
		renombrarArchivo(archivoARenombrar->rutaDeArchivo,archivoARenombrar->nuevaRuta);

		//Libero las estructuras utilizadas
		free(archivoARenombrar->nuevaRuta);
		free(archivoARenombrar->rutaDeArchivo);
		free(archivoARenombrar);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;
	}

	case LISTAR_ARCHIVOS :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeListarArchivosPokedexClient_PokedexServer* archivosListados = malloc(sizeof(t_MensajeListarArchivosPokedexClient_PokedexServer));

		//Deserializo el mensaje recibido

		deserializarMensajeListarArchivos(bufferRecibido,archivosListados);

		//Se procede a listar los archivos del directorio


		//printf("%s \n", archivosListados->rutaDeArchivo);


		listarArchivos(archivosListados->rutaDeArchivo,socketCliente);

		//Se envia al cliente los resultados

		//Se envia el tamaño de la estructura
		//enviar(socketCliente,tamanioListado,sizeof(int));

		//Se envian los archivos listados en un char* separados por un /0
		//enviar(socketCliente,listado,tamanioListado);

		//Libero las estructuras utilizadas
		free(archivosListados->rutaDeArchivo);
		free(archivosListados);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;
	}

	case TRUNCAR_ARCHIVO :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeTruncarArchivoPokedexClient_PokedexServer* archivoTruncar = malloc(sizeof(t_MensajeTruncarArchivoPokedexClient_PokedexServer));

		//Deserializo el mensaje recibido

		deserializarMensajeTruncarArchivo(bufferRecibido,archivoTruncar);

		//Se procede a renombrar archivo

		truncarArchivo(archivoTruncar->rutaDeArchivo,archivoTruncar->nuevoTamanio);

		//Libero las estructuras utilizadas
		free(archivoTruncar->rutaDeArchivo);
		free(archivoTruncar);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;

	}
	case MOVER_ARCHIVO :{

		//Reservo espacio para la estructura a utilizar

		t_MensajeMoverArchivoPokedexClient_PokedexServer* archivoMover = malloc(sizeof(t_MensajeMoverArchivoPokedexClient_PokedexServer));

		//Deserializo el mensaje recibido
		deserializarMensajeMoverArchivo(bufferRecibido,archivoMover);

		//Se procede a mover el archivo
		moverArchivo(archivoMover->rutaDeArchivo, archivoMover->nuevaRuta);

		//Libero las estructuras utilizadas
		free(archivoMover->nuevaRuta);
		free(archivoMover->rutaDeArchivo);
		free(archivoMover);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;

	}
	case ATRIBUTO_ARCHIVO :{

		//Reservo espacio para la estructura a utilizar
		t_MensajeAtributosArchivoPokedexClient_PokedexServer* atributoArchivo = malloc(sizeof(t_MensajeAtributosArchivoPokedexClient_PokedexServer));

		//Deserializo el mensaje
		deserializarMensajeAtributosArchivo(bufferRecibido, atributoArchivo);

		//Se obtienen los atributos del archivo
		atributosArchivo(atributoArchivo->rutaArchivo, socketCliente);

		//Libero las estructuras utilizadas
		free(atributoArchivo->rutaArchivo);
		free(atributoArchivo);
		free(operacionYtamanio);
		free(bufferRecibido);

		break;

	}
	default :{
		printf("Operacion no valida \n");

	}

	}

}

char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo){
	char** arrayDeRuta = string_split(rutaDeArchivoNuevo, "/");
	char* nombreDeArchivo;
	int i = 0;
	while(arrayDeRuta[i+1]!= NULL){

		i++;
	}

	int tamanio = string_length(arrayDeRuta[i]);
	nombreDeArchivo = malloc(tamanio);

	strcpy(nombreDeArchivo, arrayDeRuta[i]);

	return nombreDeArchivo;

}
char* nombreDeRutaNueva(char* rutaDeArchivoNuevo){
	char** arrayDeRuta = string_split(rutaDeArchivoNuevo, "/");
	char* nombreDeArchivo = string_new();
	int i = 0;
	while(arrayDeRuta[i+1]!= NULL){

		i++;
	}
	strcpy(nombreDeArchivo, arrayDeRuta[i]);

	return nombreDeArchivo;

}


int cantidadDeBloquesVacios(){

	int i;
	int cantidadDeBloquesVacios = 0;
	for(i=0 ; i < disco->header.fs_blocks ; i++){

		if(bitarray_test_bit(disco->bitmap, i) == 0){

			cantidadDeBloquesVacios++;

		}

	}
	return cantidadDeBloquesVacios;

}

void borrarBloqueDeDatosEnElBitmap(int posicionBloque){

	int posicionEnElBitmap = disco->header.data_blocks + posicionBloque;
	bitarray_clean_bit(disco->bitmap, posicionEnElBitmap);

}


void eliminarUltimoBloqueDeArchivo(int posicionArchivoATruncar){

	int i=disco->tablaDeArchivos[posicionArchivoATruncar].first_block;
	int posicionAnterior = i;
	while(disco->tablaDeAsignaciones[i]!=ULTIMO_BLOQUE){
		posicionAnterior = i;
		i=disco->tablaDeAsignaciones[i];


	}

	borrarBloqueDeDatosEnElBitmap(disco->tablaDeAsignaciones[posicionAnterior]);
	disco->tablaDeAsignaciones[posicionAnterior] = ULTIMO_BLOQUE;




}


int ultimaPosicionBloqueDeDatos(osada_file archivo){
	int* secuencia;
	double tamanioAReservar = ceil((double)archivo.file_size / (double)OSADA_BLOCK_SIZE);
	secuencia = malloc(tamanioAReservar * sizeof(int));
	int i = archivo.first_block;
	secuencia[0]=archivo.first_block;
	int j=1;

	while(disco->tablaDeAsignaciones[i]!= ULTIMO_BLOQUE){
		secuencia[j] = disco->tablaDeAsignaciones[i];
		i=disco->tablaDeAsignaciones[i];
		j++;
				}

	return disco->tablaDeAsignaciones[i];

}


void mapearHeader(FILE* archivoAbierto){

	int descriptorArchivo;
	descriptorArchivo = fileno(archivoAbierto);
	lseek(descriptorArchivo, 0, SEEK_SET);
	void* archivoMapeado=malloc(OSADA_BLOCK_SIZE);
	archivoMapeado=mmap(NULL, OSADA_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, descriptorArchivo, 0);
	memcpy(&disco->header,archivoMapeado,OSADA_BLOCK_SIZE);

}

void mapearBitmap(FILE* archivoAbierto){

	int descriptorArchivo;
	descriptorArchivo = fileno(archivoAbierto);
	lseek(descriptorArchivo, 0, SEEK_SET);
	char* bitmap=malloc(OSADA_BLOCK_SIZE*disco->header.bitmap_blocks);
	bitmap= (char*)mmap(NULL, OSADA_BLOCK_SIZE*disco->header.bitmap_blocks, PROT_READ | PROT_WRITE, MAP_SHARED, descriptorArchivo, disco->header.bitmap_blocks*OSADA_BLOCK_SIZE);
	disco->bitmap = bitarray_create(bitmap,disco->header.bitmap_blocks*OSADA_BLOCK_SIZE*8);

}

void mapearTablaDeArchivos(FILE* archivoAbierto){

	int descriptorArchivo;
	int i;
	descriptorArchivo = fileno(archivoAbierto);
	lseek(descriptorArchivo, 0, SEEK_SET);
	int offset=0;

	for(i=0;i<2048;i++){
		osada_file* aux= malloc(OSADA_BLOCK_SIZE/2);
		aux=(osada_file*)mmap(NULL,sizeof(osada_file), PROT_READ | PROT_WRITE, MAP_SHARED, descriptorArchivo, (1+disco->header.bitmap_blocks)*OSADA_BLOCK_SIZE+offset);

		printf("%i \n",aux->file_size);
		printf("%s \n",aux->fname);
		printf("%i \n",aux->lastmod);
		printf("%i \n",aux->state);





		memcpy(&(disco->tablaDeArchivos[i].state),aux, OSADA_BLOCK_SIZE/2);
		free(aux);

		offset+=32;
	}



}

void mapearTablaDeAsignaciones(FILE* archivoAbierto){
	int descriptorArchivo;
	descriptorArchivo = fileno(archivoAbierto);
	lseek(descriptorArchivo, 0, SEEK_SET);
	int tamanioTabla=(disco->header.fs_blocks-1-disco->header.bitmap_blocks-1024)*4/OSADA_BLOCK_SIZE;
	disco->tablaDeAsignaciones=malloc(tamanioTabla);
	disco->tablaDeAsignaciones = mmap(NULL, tamanioTabla, PROT_READ | PROT_WRITE, MAP_SHARED, descriptorArchivo, disco->header.allocations_table_offset);
}

void mapearBloquesDeDatos(FILE* archivoAbierto){
	int descriptorArchivo;
	descriptorArchivo = fileno(archivoAbierto);
	lseek(descriptorArchivo, 0, SEEK_SET);
	int tamanioTablaAsignaciones=(disco->header.fs_blocks-1-disco->header.bitmap_blocks-1024)*4/OSADA_BLOCK_SIZE;
	int tamanioBloqueDeDatos=(disco->header.fs_blocks-disco->header.bitmap_blocks-tamanioTablaAsignaciones-1-1024)*OSADA_BLOCK_SIZE;
	disco->bloquesDeDatos = mmap(NULL, tamanioBloqueDeDatos, PROT_READ | PROT_WRITE, MAP_SHARED, descriptorArchivo, disco->header.allocations_table_offset+tamanioTablaAsignaciones);
}

void mapearEstructura(void* discoMapeado){

	// SE CARGA EL HEADER DEL DISCO POR REFERENCIA

	int offset=0;

	memcpy(&disco->header, discoMapeado+offset , OSADA_BLOCK_SIZE);
	offset+=OSADA_BLOCK_SIZE;
	//printf("%i\n",disco->header.bitmap_blocks);
	//printf("%i\n",disco->header.data_blocks);
	//printf("%i\n",disco->header.version);
	//printf("%i\n",disco->header.fs_blocks);
	//printf("%i\n",disco->header.allocations_table_offset);


	// SE CARGA EL BITMAP DEL DISCO

	char* bitmap=malloc(OSADA_BLOCK_SIZE*disco->header.bitmap_blocks);
	memcpy(bitmap, discoMapeado + offset, OSADA_BLOCK_SIZE * disco->header.bitmap_blocks);
	disco->bitmap = bitarray_create(bitmap, disco->header.bitmap_blocks * OSADA_BLOCK_SIZE);
	offset+= disco->header.bitmap_blocks * OSADA_BLOCK_SIZE ;


	// SE CARGA LA TABLA DE ARCHIVOS DEL DISCO

	int i;

	for(i = 0; i < 2048; i++){

		memcpy(&disco->tablaDeArchivos[i].state, discoMapeado+offset, sizeof(char));
		offset+=sizeof(char);
		//printf("Estado: %i \n", disco->tablaDeArchivos[i].state);

		memcpy(&disco->tablaDeArchivos[i].fname, discoMapeado+offset, 17);
		offset+=17;
		//printf("Nombre: %s \n", disco->tablaDeArchivos[i].fname);

		memcpy(&disco->tablaDeArchivos[i].parent_directory, discoMapeado+offset, 2);
		offset+=2;
		//printf("Directorio padre: %i \n", disco->tablaDeArchivos[i].parent_directory);

		memcpy(&disco->tablaDeArchivos[i].file_size,discoMapeado + offset, sizeof(int));
		offset+=sizeof(int);
		//printf("Tamaño de archivo: %i \n", disco->tablaDeArchivos[i].file_size);

		memcpy(&disco->tablaDeArchivos[i].lastmod,discoMapeado+offset, sizeof(int));
		offset+=sizeof(int);
		//printf("Ultima modificacion: %i \n", disco->tablaDeArchivos[i].lastmod);

		memcpy(&disco->tablaDeArchivos[i].first_block,discoMapeado+offset, sizeof(int));
		offset+=sizeof(int);
		//printf("Primer bloque: %i \n", disco->tablaDeArchivos[i].first_block);

		/*			//Para el test.
					if(disco->tablaDeArchivos[i].state == DIRECTORY){

						printf("Nombre del directorio: %s \n  La posicion en la tabla de archivos es: %i \n Directorio Padre: %i \n El tamaño del directorio es: %i \n \n", disco->tablaDeArchivos[i].fname, i, disco->tablaDeArchivos[i].parent_directory, disco->tablaDeArchivos[i].file_size);

					}
					if(disco->tablaDeArchivos[i].state == REGULAR){

						printf("Nombre del archivo: %s \n Tamaño archivo: %i \n La posicion en la tabla de archivos es: %i \n Directorio Padre: %i \n \n", disco->tablaDeArchivos[i].fname, disco->tablaDeArchivos[i].file_size , i , disco->tablaDeArchivos[i].parent_directory);

					}
		 */
	}

	//SE CARGA LA TABLA DE ASIGNACIONES


	offset=(disco->header.allocations_table_offset) * OSADA_BLOCK_SIZE;

	//printf("El bloque donde comienza la tabla de asignaciones es: %d \n", bloqueComienzoTablaDeAsignaciones );
	//printf("El bloque donde comienza la tabla de asignaciones es: %d \n", disco->header.allocations_table_offset);


	int cantidadDeEnteros = disco->header.data_blocks;

	int tamanioTablaDeAsignaciones = cantidadDeEnteros*4;


	disco->tablaDeAsignaciones= malloc(tamanioTablaDeAsignaciones);

	int z;

	for(z = 0; z < cantidadDeEnteros; z++){


		memcpy(&disco->tablaDeAsignaciones[z],discoMapeado + offset, sizeof(int));
		//printf("%i \n", disco->tablaDeAsignaciones[z]);
		offset+=sizeof(int);


	}






	//SE CARGAN LOS BLOQUES DE DATOS

	offset=(disco->header.fs_blocks - disco->header.data_blocks)*OSADA_BLOCK_SIZE;

	int tamanioBloquesDeArchivos = disco->header.data_blocks * OSADA_BLOCK_SIZE;
	int cantidadBloquesDeDatos = disco->header.data_blocks;
	int j;

	disco->bloquesDeDatos=malloc(tamanioBloquesDeArchivos);

	for(j=0; j < cantidadBloquesDeDatos; j++){


		memcpy(disco->bloquesDeDatos[j] , discoMapeado + offset, OSADA_BLOCK_SIZE);
		offset+=OSADA_BLOCK_SIZE;


	}


	//printf("El tamaño del disco es: %d \n", offset);
	//printf("El tamaño del disco es: %d \n", disco->header.fs_blocks * OSADA_BLOCK_SIZE);
}

void inicializarSemaforos(){

	int i;
	for(i = 0; i < 2048; i++){

		sem_init(&semaforos_permisos[i],0,1);



	}



}

void destruirSemaforos(){

	int i;
	for(i = 0; i < 2048; i++){

		sem_destroy(&semaforos_permisos[i]);


	}

}
t_list* crearListaDeSecuencia(osada_file archivo){

	t_list* listaSecuencia = list_create();

	double cantidadElementos = ceil((double)archivo.file_size / (double)OSADA_BLOCK_SIZE);

	list_add(listaSecuencia,archivo.first_block);
	int i = archivo.first_block;

	int j=1;
	while(disco->tablaDeAsignaciones[i]!= -1){
		list_add(listaSecuencia, disco->tablaDeAsignaciones[i]);
		i=disco->tablaDeAsignaciones[i];
			//printf("secuencia[%i]: %i \n",j, (int)list_get(listaSecuencia,j));
			//printf("Siguiente posicion en la Tabla de asignaciones: %i \n", disco->tablaDeAsignaciones[i]);
		j++;
		}


		//printf("secuencia[%i]: %i \n",j, secuencia[j]);
		return listaSecuencia;



}

void destruirEntero(int* puntero){
	free(puntero);

}




/////////////////////////////////////////////////FUNCIONES DE CONEXIONES///////////////////////////////////////////////////////

void clienteNuevo(void* parametro){
	t_server* datosServer = malloc(sizeof(t_server));
	memcpy(&datosServer->socketServer, parametro, sizeof(datosServer->socketServer));
	pthread_attr_t hiloDeAceptarConexiones;
	pthread_attr_init(&hiloDeAceptarConexiones);
	pthread_attr_setdetachstate(&hiloDeAceptarConexiones, PTHREAD_CREATE_DETACHED);
	pthread_t hiloDeAceptarClientes;
	pthread_create(&hiloDeAceptarClientes, &hiloDeAceptarConexiones, (void*) aceptarConexionDeUnClienteHilo, &datosServer);
	pthread_attr_destroy(&hiloDeAceptarConexiones);
	aceptarConexionDeUnCliente(&datosServer->socketCliente, &datosServer->socketServer);
	while(1){
		escucharOperaciones(&datosServer->socketCliente);
	}
}

void startServer() {
	int socketSv = 0;
	abrirConexionDelServer(conexion.ip, conexion.puerto, &socketSv);
	while (1) {
		clienteNuevo((void*) &socketSv);
	}
}


