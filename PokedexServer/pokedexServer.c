/*
 * pokedexServer.c
 *
 */

#include "pokedexServer.h"


int main(int argc, char **argv) {

	//char *logFile = NULL;
	//inicializarBloqueCentral();
	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	/*//Parametros
	int i;
	for( i = 0; i < argc; i++){
		if (string_equals_char **argvignore_case(argv[i], "") == 0){
			logFile = argv[i+1];
			printf("Log File: '%s'\n",logFile);
		}
	}*/

	//Creo el archivo de Log
	//logPokedex = log_create(logFile, "POKEDEXCLIENT", 0, LOG_LEVEL_TRACE);




	/*
	startServer();


	FILE* discoAbierto = fopen(rutaDisco,"r+");

	disco = (osada_bloqueCentral*)mapearArchivoMemoria(discoAbierto);

	return 0;
	*/


	//memset(buffer,0,tamaniobuffer);
	/*
	size_t offset=0;
	memset(buffer,'1',tamaniobuffer);
	memcpy(buffer+offset,&pruebaEscribir->cantidadDeBytes,sizeof( int));
	offset+=sizeof(int);
	memcpy(buffer+offset,&pruebaEscribir->rutaArchivo,tamanioRuta);
	offset+=tamanioRuta;
	memcpy(buffer+offset,&pruebaEscribir->bufferAEscribir,pruebaEscribir->cantidadDeBytes);
	offset+=pruebaEscribir->cantidadDeBytes;
	memcpy(buffer+offset,&pruebaEscribir->offset,sizeof(int));

	t_MensajeEscribirArchivoPokedexClient_PokedexServer* mensajeDeserializado=malloc(tamaniobuffer);
	memcpy(&(mensajeDeserializado->cantidadDeBytes),buffer,sizeof(int));
	memcpy(&(mensajeDeserializado->rutaArchivo),buffer+4,tamanioRuta);
	memcpy(&(mensajeDeserializado->bufferAEscribir),buffer+4+tamanioRuta,mensajeDeserializado->cantidadDeBytes);
	memcpy(&(mensajeDeserializado->offset),buffer+8+tamanioRuta,sizeof(int));
	*/


	///////Prueba serializadores LOCALHOST SERVER ////////////////////

	conexion.ip="127.0.0.1";
		conexion.puerto=7000;

		int socket = ponerAEscuchar(conexion.ip,conexion.puerto);

		void* recibirBufferYOperacion = malloc(sizeof(int) * 2);

		recibir(&socket, recibirBufferYOperacion, sizeof(int)*2);

		t_pedidoPokedexCliente* operacionARealizar = malloc(sizeof(int)*2);


		deserializarOperaciones(recibirBufferYOperacion,operacionARealizar);

		printf("%i \n", operacionARealizar->operacion);
		printf("%i \n", operacionARealizar->tamanioBuffer);


		void* bufferARecibir= malloc(operacionARealizar->tamanioBuffer);

		t_MensajeEscribirArchivoPokedexClient_PokedexServer* escribirArchivo;
		escribirArchivo = malloc(sizeof(t_MensajeEscribirArchivoPokedexClient_PokedexServer));

		recibir(&socket, bufferARecibir, operacionARealizar->tamanioBuffer);

		deserializarMensajeEscribirOModificarArchivo(bufferARecibir,escribirArchivo);

		printf("%d \n", escribirArchivo->tamanioRuta);
		printf("%s \n", escribirArchivo->rutaArchivo);
		printf("%s \n", escribirArchivo->bufferAEscribir);
		printf("%d \n", escribirArchivo->cantidadDeBytes);
		printf("%d \n", escribirArchivo->offset);



		return 0;


}

////////////////////////////FUNCIONES PROPIAS DEL FILESYSTEM/////////////////////////////////////
void leerArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* buffer){

	//Busco el archivo en mi tabla de Archivos
	osada_file archivoALeer=buscarArchivoPorRuta(rutaArchivo);

	//Armo mi secuencia de bloques usando la tabla de asginaciones
	int* secuenciaDeBloqueALeer=buscarSecuenciaBloqueDeDatos(archivoALeer);

	//Calculo la cantidad de bloque de datos
	int cantidadDeBloques=calcularBloquesAPedir(cantidadDeBytes);

	//Alloco memoria en un char*
	char* archivoCompleto=malloc(cantidadDeBloques*OSADA_BLOCK_SIZE);

	//Busco los bloques de datos y los copio en una solo puntero
	int i=0;
	while(secuenciaDeBloqueALeer[i]!=NULL){
		memcpy(archivoCompleto,disco->bloquesDeDatos[i],OSADA_BLOCK_SIZE);
	}
	//Del archivo toma la cantidad de bytes que me pidieron desde donde me pidieron

	memcpy(buffer,archivoCompleto+offset,cantidadDeBytes);
	//Enviar al cliente la seccion del archivo que pidio


}

void crearArchivo(char* rutaArchivoNuevo){

	//Reservo memoria para el nombre del archivo
	unsigned char nombreDeArchivo[17];

	//Sacar nombre del archivo de la ruta obtenida
	strcpy(nombreDeArchivo,nombreDeArchivoNuevo(rutaArchivoNuevo));



	//Posicion del directorio padre

		int posicionDirectorioPadre = posicionArchivoPorRuta(rutaArchivoNuevo);

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
				if(string_equals_ignore_case(disco->tablaDeArchivos[i].fname, nombreDeArchivo)){
					printf("No se puede crear archivo. Nombre de archivo existente");
					return;
					//Informar cliente que ya existe un archivo con este nombre
				}

			}
		}

	//Buscar un Osada File vacio en la tabla de Archivos
		int osadaFileVacio = 0;
			while(disco->tablaDeArchivos[osadaFileVacio].state != DELETED ){
						osadaFileVacio++;
					}


	////////////////SETEO DE ARCHIVO//////////////////////////


	//Inicializacion de tiempo de creacion

	time_t tiempo;
	struct tm* tm;
	disco->tablaDeArchivos[osadaFileVacio].lastmod=tm->tm_mday*10000+tm->tm_mon*100+tm->tm_year;
	tiempo=time(NULL);
	tm=localtime(&tiempo);

	//Seteo nombre de archivo
	string_equals_ignore_case(&disco->tablaDeArchivos[osadaFileVacio].fname,&nombreDeArchivo);

	//Seteo estado nuevo del bloque
	disco->tablaDeArchivos[osadaFileVacio].state = REGULAR;

	//Seteo bloque Padre
	disco->tablaDeArchivos[osadaFileVacio].parent_directory = posicionDirectorioPadre;

	//Seteo tamaño inicial del archivo -> como esta vacio sera del tamaño minimo de un bloque OSADA_BLOCK_SIZE
	disco->tablaDeArchivos[osadaFileVacio].file_size = OSADA_BLOCK_SIZE;

	//Seteo bloque inicial del archivo
	disco->tablaDeArchivos[osadaFileVacio].first_block = bloqueVacio;

}



/////////////////// ESCRIBIR O MODIFICAR ARCHIVO ///////////////////

void escribirOModificarArchivo(char* rutaArchivo,int offset,int cantidadDeBytes,char* bufferAEscribir){

	//Se busca el archivo que es unico en todo el FileSystem
	osada_file archivoAEscribir = buscarArchivoPorRuta(rutaArchivo);


	// Calculo la cantidad de Bloques del archivo en el File System
	double cantidadBloques = ceil(archivoAEscribir.file_size / OSADA_BLOCK_SIZE);

	//Busco la secuencia de bloques de mi archivo
	int* secuenciaDeBloques=buscarSecuenciaBloqueDeDatos(archivoAEscribir);

	//Busco dentro de la secuencia el o los bloques a escribir basandome en el offset
	int bloqueComienzoEscritura=ceil(offset/OSADA_BLOCK_SIZE);
	int byteDelBloque=offset%OSADA_BLOCK_SIZE;
	int cantidadDeBytesDisponible=OSADA_BLOCK_SIZE-byteDelBloque;
	int cantidadDeBloquesNecesarios=ceil(cantidadDeBytes/OSADA_BLOCK_SIZE);

	//Verifico que exista bloques de datos disponibles
	int bloquesVacios = cantidadDeBloquesVacios();
	if(cantidadDeBloquesNecesarios < bloquesVacios){
		printf("No se encuentran bloques vacios");
		//Comunicarse con el cliente y avisarle que no hay espacio

		return;
	}


	//Escribo verificando si entra en el mismo bloque que arranca o tengo que asignarle mas
	if(cantidadDeBytes<=cantidadDeBytesDisponible){
		memcpy(disco->bloquesDeDatos[secuenciaDeBloques[bloqueComienzoEscritura]]+offset,bufferAEscribir,cantidadDeBytesDisponible);
	}else{
		memcpy(disco->bloquesDeDatos[secuenciaDeBloques[bloqueComienzoEscritura]]+offset,bufferAEscribir,cantidadDeBytesDisponible);
		int j;
		int h=1;
		int ultimoBloque = calcularUltimoBloque(secuenciaDeBloques);
		for(j=0;j<cantidadDeBloquesNecesarios;j++){

			int posicionBloqueVacio = buscarBloqueVacioEnElBitmap();
			disco->tablaDeAsignaciones[ultimoBloque] = posicionBloqueVacio;
			ultimoBloque = posicionBloqueVacio;
			disco->tablaDeAsignaciones[posicionBloqueVacio] = ULTIMO_BLOQUE;

			memcpy(disco->bloquesDeDatos[secuenciaDeBloques[bloqueComienzoEscritura]+h],bufferAEscribir+cantidadDeBytesDisponible+OSADA_BLOCK_SIZE*h,OSADA_BLOCK_SIZE);
			h++;
		}
	}



}

/////////////////// BORRAR ARCHIVOS ///////////////////


void borrarArchivos(char* rutaDeArchivo){

	//Se busca el archivo que es unico en todo el FileSystem
	osada_file archivoABorrar = buscarArchivoPorRuta(rutaDeArchivo);

	//Se busca la posicion en la tabla de archivos del Archivo por la ruta dada
	int posicionDelArchivoABorrar = posicionArchivoPorRuta(rutaDeArchivo);

	// Calculo la cantidad de Bloques del archivo en el File System
	double cantidadBloques = ceil(archivoABorrar.file_size / OSADA_BLOCK_SIZE);

	// Se arma una secuencia con las direcciones del archivo
	int * secuenciaArchivo = malloc(cantidadBloques* sizeof(int));
	secuenciaArchivo = buscarSecuenciaBloqueDeDatos(archivoABorrar);


	// Se inicia el proceso de borrado de archivo poniendo en 0 el bitmap y el estado de la tabla de Archivos
	// Se pone en 0 la secuencia del archivo en el BitArray demostrando que los bloques de datos ya estan disponible para sobreescribir
	int i = 0;
	while(secuenciaArchivo[i]!= NULL){

		bitarray_clean_bit(disco->bitmap, secuenciaArchivo[i]);


	}
	// Se cambia el estado del archivo a Borrar a DELETED en la tabla de archivos
	disco->tablaDeArchivos[posicionDelArchivoABorrar].state = DELETED;
}

void crearDirectorio(char* rutaDirectorioPadre){


	//Declaracion para el nombre del archivo
	char* nombreRuta = malloc(sizeof(char)*18);

	//Sacar nombre del archivo de la ruta obtenida
	nombreRuta = nombreDeRutaNueva(rutaDirectorioPadre);

	//Se extrae la posicion del directorio padre
			int posicionDelDirectorioPadre = posicionArchivoPorRuta(rutaDirectorioPadre);

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

		memcpy(archivoARenombrar.fname, nuevoNombre, resultadoCantidadDeCaracteres );

	}

}

/////////LISTAR ARCHIVOS////////

void listarArchivos(char* rutaDirectorio){

	//Saco la posicion del directorio
	int posicionDirectorio = posicionArchivoPorRuta(rutaDirectorio);

	//Inicializo las variables
	int posicionTablaDeArchivos;
	int cantidadDeArchivos = 0;

	//Reviso cuantos archivos se encuentran en el directorio
	for(posicionTablaDeArchivos = 0; posicionTablaDeArchivos < 2047; posicionTablaDeArchivos++){

		if(disco->tablaDeArchivos[posicionTablaDeArchivos].parent_directory == posicionDirectorio){

			cantidadDeArchivos++;

		}

	//Asigno 18 bytes por la cantidad de archivos encontrados ya que cada uno sera un unsigned char[17]
	char* buffer = malloc(18*cantidadDeArchivos);

	//Asigno el nombre de los archivos y directorios encontrados al buffer a enviar y seteo un offset en 0 para avanzar en memcpy

	int offset=0;


	for(posicionTablaDeArchivos = 0; posicionTablaDeArchivos < 2047; posicionTablaDeArchivos++){

		if(disco->tablaDeArchivos[posicionTablaDeArchivos].parent_directory == posicionDirectorio){

				//Se copian en memoria los nombres de los directorios y archivos encontrados
				//en la direccion de la tabla de archivos
				memcpy(buffer + offset, &disco->tablaDeArchivos[posicionTablaDeArchivos].fname, sizeof(18));
				offset += 18;

				}



	}

	//Se envia al cliente el string obtenido





}
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
	int i;
	for(i=0;i<disco->header.bitmap_blocks;i++){
		if(bitarray_test_bit(disco->bitmap,i)==0){
			bitarray_set_bit(disco->bitmap,i);
			return i;
		}

	}
	return -1;
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
	char** arrayDeRuta = string_split(rutaAbsolutaArchivo, '/');
	int i = 1;
	int directorioInicial;
	int j=0;
	int k=0;
	int directorioAnterior;
	while(arrayDeRuta[0]!=disco->tablaDeArchivos[j].fname){
		j++;
	}
	directorioInicial=j;
	while(arrayDeRuta[i]!=NULL){
		while(arrayDeRuta[i]!=disco->tablaDeArchivos[k].fname && (disco->tablaDeArchivos[k].parent_directory!=directorioAnterior || disco->tablaDeArchivos[k].parent_directory!=directorioInicial)){
			k++;
			directorioAnterior=disco->tablaDeArchivos[k].parent_directory;
		}
		i++;
	}
	return disco->tablaDeArchivos[k];
}



int* buscarSecuenciaBloqueDeDatos(osada_file archivo){
	int* secuencia;
	double tamanioAReservar = ceil(archivo.file_size / OSADA_BLOCK_SIZE);
	secuencia = malloc(tamanioAReservar * sizeof(int));
	int i = archivo.first_block;
	secuencia[0]=archivo.first_block;
	int j=1;
	while(disco->tablaDeAsignaciones[i]!= ULTIMO_BLOQUE){
		secuencia[j] = disco->tablaDeAsignaciones[i];
		i=disco->tablaDeAsignaciones[i];
		j++;
	}
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

	double bloquesAPedir = ceil(bytesRestantes / OSADA_BLOCK_SIZE);

	return bloquesAPedir;

}

int calcularPosicionDeEscrituraUltimoBloque(int cantidadRestanteAEscribir){

	int posicionUltimoBloque = cantidadRestanteAEscribir % OSADA_BLOCK_SIZE;

	return posicionUltimoBloque;

}

int calcularUltimoBloque(int* secuencia){

	int i = 0;

	while (secuencia[i]!= ULTIMO_BLOQUE){

		i++;
	}

	return i;

}
int revisarMismoNombre(osada_file archivoARenombrar, char* nuevoNombre){

	int i;
	for(i = 0; i < 2047; i++){

		if(disco->tablaDeArchivos[i].parent_directory == archivoARenombrar.parent_directory){
			if(string_equals_ignore_case(disco->tablaDeArchivos[i].fname,nuevoNombre)){
				printf("Error: nombre existente en este directorio");
				return 0;
			}
		}

	}

	return 1;

}
int posicionArchivoPorRuta(char* rutaAbsolutaArchivo){
	char** arrayDeRuta = string_split(rutaAbsolutaArchivo, "/");
	int i = 1;
	int directorioInicial;
	int j=0;
	int k=0;
	int directorioAnterior;
	while(arrayDeRuta[0]!=disco->tablaDeArchivos[j].fname){
		j++;
	}
	directorioInicial=j;
	while(arrayDeRuta[i]!=NULL){
		while(arrayDeRuta[i]!=disco->tablaDeArchivos[k].fname && (disco->tablaDeArchivos[k].parent_directory!=directorioAnterior || disco->tablaDeArchivos[k].parent_directory!=directorioInicial)){
			k++;
			directorioAnterior=disco->tablaDeArchivos[k].parent_directory;
		}
		i++;
	}
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

	//Reservo memoria para recibir el buffer del cliente

	void* bufferRecibido = malloc(operacionYtamanio->tamanioBuffer);
	recibir(socketCliente,bufferRecibido,operacionYtamanio->tamanioBuffer);

	printf("%i",operacionYtamanio->operacion);

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

			//Libero las estructuras utilizadas
			free(lecturaNueva);
			free(operacionYtamanio);

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
		  		free(archivoNuevo);
		  		free(operacionYtamanio);
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
		   		  		free(escrituraNueva);
		   		  		free(operacionYtamanio);

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
		   		   		  		free(borradoNuevo);
		   		   		  		free(operacionYtamanio);

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
		   		   		   		  		free(directorioNuevo);
		   		   		   		  		free(operacionYtamanio);
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
		   		   	free(directorioABorrar);
		   		   	free(operacionYtamanio);
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
		   		   		   		   		   		  		free(archivoARenombrar);
		   		   		   		   		   		  		free(operacionYtamanio);

		   break;
	   	   }
	/*
	case LISTAR_ARCHIVOS :{

		  * TERMINAR
		  *
		  *  //Reservo espacio para la estructura a utilizar

		    archivoARenombrar=malloc(tamanioDelBuffer);

		   		   //Deserializo el mensaje recibido
		   		   deserializarMensajeRenombrarArchivo(bufferRecibido,archivoARenombrar);

		   		   //Se procede a renombrar archivo
		   		   renombrarArchivo(archivoARenombrar->rutaDeArchivo,archivoARenombrar->nuevaRuta);

		   		   //Libero las estructuras utilizadas
		   		   		   		   		   		   		  		free(archivoARenombrar);
		   		   		   		   		   		   		  		free(operacionYtamanio);


	   }*/
	   default :{
	   printf("Operacion no valida \n");

	   }

	}

}

char* nombreDeArchivoNuevo(char* rutaDeArchivoNuevo){
		   char** arrayDeRuta = string_split(rutaDeArchivoNuevo, "/");
		   char* nombreDeArchivo = string_new();
		   int i = 0;
		   while(arrayDeRuta[i]!= NULL){

			   i++;
		   }
			strcpy(nombreDeArchivo, arrayDeRuta[i]);

		   return nombreDeArchivo;

}
char* nombreDeRutaNueva(char* rutaDeArchivoNuevo){
		   char** arrayDeRuta = string_split(rutaDeArchivoNuevo, "/");
		   char* nombreDeArchivo = string_new();
		   int i = 0;
		   while(arrayDeRuta[i]!= NULL){

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


int ultimaPosicionBloqueDeDatos(osada_file archivo){
	int* secuencia;
	double tamanioAReservar = ceil(archivo.file_size / OSADA_BLOCK_SIZE);
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
	//escucharOperaciones(&datosServer->socketCliente);
}

void startServer() {
	int socketSv = 0;
	abrirConexionDelServer(conexion.ip, conexion.puerto, &socketSv);
	while (1) {
		clienteNuevo((void*) &socketSv);
	}
}


