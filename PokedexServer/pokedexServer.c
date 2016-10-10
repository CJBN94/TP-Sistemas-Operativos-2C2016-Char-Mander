/*
 * pokedexServer.c
 *
 */

#include "pokedexServer.h"
#include "osada.h"


osada_header* miFileSystem;
osada_file* tablaDeArchivos[2048];
t_bitarray* mapaDeBits;

int main(int argc, char **argv) {

	char *logFile = NULL;

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
void leerArchivo(char* rutaFileSystem,char* nombreDelArchivo){

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

void EscribirOModificar(osada_file* archivoAModificar,int* tablaDeAsignaciones,char* rutaFs){

}

void borrarArchivos(){

}

void crearDirectorio(){

}

void borrarDirectoriosVacios(){

}

void renombrarArchivo(){

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


int buscarBloqueVacioEnElBitmap(){
	int i;
	for(i=0;i<miFileSystem->bitmap_blocks;i++){
		if(bitarray_test_bit(mapaDeBits,i)==0){
			bitarray_set_bit(mapaDeBits,i);
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

void seteoInicialFS(char* rutaFS){
	tamanioFileSystem=1024;
	miFileSystem->fs_blocks=tamanioFileSystem/OSADA_BLOCK_SIZE;
	miFileSystem->version=1;
	miFileSystem->bitmap_blocks=miFileSystem->fs_blocks/8/OSADA_BLOCK_SIZE;
	miFileSystem->allocations_table_offset=1+miFileSystem->bitmap_blocks+1024;
	int tamanioTablaDeAsignaciones=(miFileSystem->fs_blocks-1-miFileSystem->bitmap_blocks-1024)*4/OSADA_BLOCK_SIZE;
	miFileSystem->data_blocks=miFileSystem->fs_blocks-1-miFileSystem->bitmap_blocks-tamanioTablaDeAsignaciones;
	int tablaDeAsignaciones[miFileSystem->data_blocks];
	char* mapa=malloc(miFileSystem->bitmap_blocks/8);
	mapaDeBits=bitarray_create(mapa,miFileSystem->bitmap_blocks);
	FILE* archivoFs;
	archivoFs=fopen(rutaFS,"r+");
	int tamanioAReservar;
	tamanioAReservar=calcularTamanioDeArchivo(archivoFs);
	void* archivoMapeado=malloc(tamanioAReservar);
	archivoMapeado=mapearArchivoMemoria(archivoFs);
	//copiar las estructuras administrativas dentro del archivo del fs
}

