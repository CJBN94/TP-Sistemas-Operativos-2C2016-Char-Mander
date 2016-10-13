/*
 * pokedexServer.c
 *
 */

#include "pokedexServer.h"


int main(int argc, char **argv) {

	char *logFile = NULL;
	inicializarBloqueCentral();
	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	/*//Parametros
	int i;
	for( i = 0; i < argc; i++){
		if (strcmp(argv[i], "") == 0){
			logFile = argv[i+1];
			printf("Log File: '%s'\n",logFile);
		}
	}*/

	//Creo el archivo de Log
	//logPokedex = log_create(logFile, "POKEDEXCLIENT", 0, LOG_LEVEL_TRACE);

	/*
	char* rutaArchivoDePrueba="/home/utnso/Escritorio/PruebaDeMapeo.txt";
	FILE* archivoDePrueba=fopen(rutaArchivoDePrueba,"r+");
	int tamanio=calcularTamanioDeArchivo(archivoDePrueba);
	void* archivoMapeado=malloc(tamanio);
	archivoMapeado=mapearArchivoMemoria(archivoDePrueba);
	char* frase=malloc(15);
	memcpy(frase,archivoMapeado,15);
	printf("%s",frase);
	printf("%i",tamanio);
	 */
}


////////////////////////////FUNCIONES PROPIAS DEL FILESYSTEM/////////////////////////////////////
void leerArchivo(unsigned char nombreDelArchivo[17]){
	FILE* discoAbierto = fopen(rutaDisco,"r+");
	osada_file infoArchivo;
	disco = (osada_bloqueCentral*)mapearArchivoMemoria(discoAbierto);
	int i = 0;
	while(disco->tablaDeArchivos[i].file_size != (-1)){
		if(disco->tablaDeArchivos[i].fname == nombreDelArchivo){
			infoArchivo = disco->tablaDeArchivos[i];
		}
		i++;
	}
	int cantidadDeBloquesArchivo = ceill(infoArchivo.file_size / OSADA_BLOCK_SIZE);
	int secuenciaArchivo[cantidadDeBloquesArchivo];
	int j = infoArchivo.first_block;
	secuenciaArchivo[0] = j;
	int h = 1;
	while(disco->tablaDeAsignaciones[j] != (-1)){
		secuenciaArchivo[h] = disco->tablaDeAsignaciones[j];
		j = disco->tablaDeAsignaciones[j];
		h++;
	}
	int k = 0;
	char* archivo = malloc(cantidadDeBloquesArchivo*8);
	while(secuenciaArchivo[k] != NULL){
		memcpy(archivo+OSADA_BLOCK_SIZE*k,
				disco->bloquesDeDatos+secuenciaArchivo[k]*OSADA_BLOCK_SIZE,OSADA_BLOCK_SIZE );
		k++;
	}
	//TODO Enviar archivo al cliente
}

void crearArchivo(char* nombreArchivoNuevo,int tamanio,int directorioPadre){
	time_t tiempo;
	struct tm* tm;
	osada_file* nuevoArchivo=malloc(sizeof(osada_file));
	strcpy(&nuevoArchivo->fname,nombreArchivoNuevo);
	nuevoArchivo->file_size=tamanio;
	nuevoArchivo->first_block=-buscarBloqueVacioEnElBitmap();
	nuevoArchivo->state=REGULAR;
	tiempo=time(NULL);
	tm=localtime(&tiempo);
	nuevoArchivo->lastmod=tm->tm_mday*10000+tm->tm_mon*100+tm->tm_year;
	nuevoArchivo->parent_directory=directorioPadre;
	char* comandoCreacion=string_new();
	char* rutaPokedex;//falta definir la ruta donde se va a guardar PREGUNTAR
	comandoCreacion=string_from_format("touch %s",rutaPokedex);
	system(comandoCreacion);
	//Esto no va lo dejo porque probablemente se use en escribir o modificar archivo
	/*FILE* archivoAbierto=fopen(rutaFileSystem,"r+");
	int tamanioAReservar;
	tamanioAReservar=calcularTamanioDeArchivo(archivoAbierto);
	int cantidadDeBloquesArchivos=ceil(tamanio/OSADA_BLOCK_SIZE);
	completarTablaDeAsignaciones(tablaDeAsignaciones,cantidadDeBloquesArchivos,nuevoArchivo->first_block);
	void* archivoMapeado=malloc(tamanioAReservar);
	archivoMapeado=mapearArchivoMemoria(archivoAbierto);
	 */
}


void crearArchivo2(char* direccionDisco){



}

/////////////////// ESCRIBIR O MODIFICAR ARCHIVO ///////////////////

void EscribirOModificar(char* rutaArchivo,char* loQueVoyAEscribir){

	// abrimos el archivo de disco
	FILE* discoAbierto = fopen(rutaDisco, "r+");

	//Se mapea el disco a memoria
	disco =(osada_bloqueCentral*) mapearArchivoMemoria(discoAbierto);

	// Calculo el tamaño de lo que se va a escribir
	int tamanioDeEscritura = string_length(loQueVoyAEscribir);

	//Se busca el archivo que es unico en todo el FileSystem
	osada_file archivoAEscribir = buscarArchivoPorRuta(rutaArchivo);

	// Calculo la cantidad de Bloques del archivo en el File System
	double cantidadBloques = ceil(archivoAEscribir.file_size / OSADA_BLOCK_SIZE);

	// Se arma una secuencia con las direcciones del archivo
	int * secuenciaArchivo = malloc(cantidadBloques* sizeof(int));
	secuenciaArchivo = buscarSecuenciaBloqueDeDatos(archivoAEscribir);

	// Se ubica el ultimo bloque de memoria y se verifica si tiene espacio libre
	// para tener la menor cantidad de fragmentacion interna posible
	int cantidadEscrita = llenarEspacioLibreUltimoBloque(secuenciaArchivo,loQueVoyAEscribir);
	int cuantoLlevoEscrito = tamanioDeEscritura - cantidadEscrita;
	int posicionDeEscrituraUltimoBloque = calcularPosicionDeEscrituraUltimoBloque(cuantoLlevoEscrito);

	// Esta funcion nos devuelve la cantidad de bloques extras necesarios para finalizar la escritura
	double bloquesExtrasNecesarios = calcularBloquesAPedir(cuantoLlevoEscrito);
	int i;
	int offset=cantidadEscrita;
	int nuevoBloque;

	// Escribo los nuevos bloques de memoria
	for(i=0;i<bloquesExtrasNecesarios;i++){
	nuevoBloque = buscarBloqueVacioEnElBitmap();

	// Modifico la tabla de asignaciones para combinar los nuevos bloques asignados.
	int ultimoBloque = calcularUltimoBloque(secuenciaArchivo);
	disco->tablaDeAsignaciones[ultimoBloque] = nuevoBloque;

	// Se le asigna como ultimo bloque al nuevo bloque

	disco->tablaDeAsignaciones[nuevoBloque] = ULTIMO_BLOQUE;

	//Cargo los bloques en memoria
	memcpy(disco->bloquesDeDatos+nuevoBloque*OSADA_BLOCK_SIZE,loQueVoyAEscribir+offset,OSADA_BLOCK_SIZE);
	offset+=OSADA_BLOCK_SIZE;
	}

	// Se llena con un caracter distintivo que marque el comienzo de una proxima escritura con un '/0'
	int posicionALlenarDeBasura = posicionDeEscrituraUltimoBloque -1;
	int byteInicioBloque=nuevoBloque*OSADA_BLOCK_SIZE;
	for(posicionALlenarDeBasura; posicionALlenarDeBasura < OSADA_BLOCK_SIZE - 1; posicionALlenarDeBasura ++){
		disco->bloquesDeDatos[byteInicioBloque+posicionALlenarDeBasura] = '/0';
	}
	// Se baja a disco las modificaciones realizadas
	int tamanioDisco = calcularTamanioDeArchivo(discoAbierto);
	munmap(disco, tamanioDisco);
	fclose(discoAbierto);

}

void borrarArchivos(char* rutaDeArchivo){
	// abrimos el archivo de disco
		FILE* discoAbierto = fopen(rutaDisco, "r+");

	//Se mapea el disco a memoria
		disco =(osada_bloqueCentral*) mapearArchivoMemoria(discoAbierto);

	//Se busca el archivo que es unico en todo el FileSystem
		osada_file archivoABorrar = buscarArchivoPorRuta(rutaDeArchivo);

	// Calculo la cantidad de Bloques del archivo en el File System
		double cantidadBloques = ceil(archivoABorrar.file_size / OSADA_BLOCK_SIZE);

	// Se arma una secuencia con las direcciones del archivo
		int * secuenciaArchivo = malloc(cantidadBloques* sizeof(int));
		secuenciaArchivo = buscarSecuenciaBloqueDeDatos(archivoABorrar);


	// Se inicia el proceso de borrado de archivo poniendo en 0 el bitmap y el estado de la tabla de Archivos
		// Se pone en 0 la secuencia del archivo en el BitArray demostrando que ya esta disponible para sobreescribir
		int i = 0;
		while(secuenciaArchivo[i]!= NULL){

			bitarray_clean_bit(disco->bitmap, secuenciaArchivo[i]);


		}
		// Se cambia el estado del archivo a Borrar a DELETED
		archivoABorrar.state = DELETED;
	// Se baja a disco las modificaciones realizadas
	int tamanioDisco = calcularTamanioDeArchivo(discoAbierto);
	munmap(disco, tamanioDisco);
	fclose(discoAbierto);


}

void crearDirectorio(){

}

void borrarDirectoriosVacios(){

}

void renombrarArchivo(char* rutaDeArchivo, char* nuevoNombre){

	// abrimos el archivo de disco
			FILE* discoAbierto = fopen(rutaDisco, "r+");

	//Se mapea el disco a memoria
			disco =(osada_bloqueCentral*) mapearArchivoMemoria(discoAbierto);


	//Se busca el archivo que es unico en todo el FileSystem
			osada_file archivoARenombrar = buscarArchivoPorRuta(rutaDeArchivo);

	//Se busca si un archivo en el directorio contiene el mismo nombre
			int resultado = revisarMismoNombre(archivoARenombrar, nuevoNombre);

	//Si no se encuentra un archivo con el mismo nombre en el directorio padre se procede a cambiar el nombre
			if(resultado){

				strcpy(archivoARenombrar.fname, nuevoNombre );

			}


	// Se baja a disco las modificaciones realizadas
			int tamanioDisco = calcularTamanioDeArchivo(discoAbierto);
			munmap(disco, tamanioDisco);
			fclose(discoAbierto);




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
	int cantidadDeBits = cantidadDeBloquesTotal / 8;
	int cantidadDeBytesAMalloquear = cantidadDeBits / 8;
	int fileDescriptor;
	FILE* archivoDisco = fopen(rutaDisco,'r+');
	fileDescriptor = fileno(archivoDisco);
	lseek(fileDescriptor,0,SEEK_SET);
	char* bitmap = malloc(sizeof(cantidadDeBytesAMalloquear));
	bitmap = mmap(NULL, disco->header->bitmap_blocks , PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fileDescriptor, OSADA_BLOCK_SIZE);
	disco->bitmap = bitarray_create(bitmap, cantidadDeBits);
	munmap(bitmap,disco->header->bitmap_blocks*OSADA_BLOCK_SIZE);
	disco=(osada_bloqueCentral*)mapearArchivoMemoria(archivoDisco);
	disco->header->version=1;
	disco->header->fs_blocks=tamanioDisco/OSADA_BLOCK_SIZE;
	disco->header->bitmap_blocks=sizeof(disco->header->fs_blocks/8);
	disco->header->allocations_table_offset=disco->header->bitmap_blocks+1025;
	int cantidadDeBloquesTablaDeAsignaciones=(tamanioDisco-1-disco->header->bitmap_blocks-1024)*4/OSADA_BLOCK_SIZE;
	disco->header->data_blocks=cantidadDeBloquesTablaDeAsignaciones-disco->header->bitmap_blocks-disco->header->fs_blocks-1-1024;
	int i;
	int bloquesAdministrativos=1+1024+disco->header->bitmap_blocks+cantidadDeBloquesTablaDeAsignaciones;
	for(i=0;i<bloquesAdministrativos;i++){
		bitarray_set_bit(disco->bitmap,i);
	}
	seteoInicialTablaDeAsignaciones(disco->tablaDeAsignaciones);
	int j;
	for(j=0;j<2048;j++){
		disco->tablaDeArchivos[j].file_size=-1;
	}
}

int buscarBloqueVacioEnElBitmap(){
	int i;
	for(i=0;i<disco->header->bitmap_blocks;i++){
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
		while(arrayDeRuta[i]!=disco->tablaDeArchivos[k].fname && (disco->tablaDeArchivos[k].parent_directory!=directorioAnterior || disco->tablaDeArchivos.parent_directory!=directorioInicial)){
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


	for(int i = 0; i < 2047; i++){

		if(disco->tablaDeArchivos[i].parent_directory == archivoARenombrar.parent_directory){
			if(string_equals_ignore_case(disco->tablaDeArchivos[i].fname,nuevoNombre)){
				printf("Error: nombre existente en este directorio");
				return 0;
			}
		}

	}

	return 1;

}
