/*
 * pokedexServer.c
 *
 */

#include "pokedexServer.h"


osada_header* miFileSystem;
osada_file* tablaDeArchivos[2048];
t_bitarray* mapaDeBits;

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

	return EXIT_SUCCESS;
	char* rutaArchivoDePrueba="/home/utnso/Escritorio/PruebaDeMapeo.txt";
	FILE* archivoDePrueba=fopen(rutaArchivoDePrueba,"r+");
	int tamanio=calcularTamanioDeArchivo(archivoDePrueba);
	void* archivoMapeado=malloc(tamanio);
	archivoMapeado=mapearArchivoMemoria(archivoDePrueba);
	char* frase=malloc(15);
	memcpy(frase,archivoMapeado,15);
	printf("%s",frase);
	printf("%i",tamanio);

}


////////////////////////////FUNCIONES PROPIAS DEL FILESYSTEM/////////////////////////////////////
void leerArchivo(char* rutaFileSystem,char* nombreDelArchivo){

}

void crearArchivo(char* rutaFileSystem,char* nombreArchivoNuevo,int tamanio,int directorioPadre){
	time_t tiempo;
	struct tm* tm;
	osada_file* nuevoArchivo=malloc(sizeof(osada_file));
	strcpy(nuevoArchivo->fname,nombreArchivoNuevo);
	nuevoArchivo->file_size=tamanio;
	nuevoArchivo->first_block=buscarPrimerBloqueVacio();
	nuevoArchivo->state=REGULAR;
	tiempo=time(NULL);
	tm=localtime(&tiempo);
	nuevoArchivo->lastmod=tm->tm_mday*10000+tm->tm_mon*100+tm->tm_year;
	nuevoArchivo->parent_directory=directorioPadre;
	//Falta agregar los cambios en los bloques del archivo real
	FILE* archivoAbierto=fopen(rutaFileSystem,"r+");
	int tamanioAReservar;
	tamanioAReservar=calcularTamanioDeArchivo(archivoAbierto);
	void* archivoMapeado=malloc(tamanioAReservar);
	archivoMapeado=mapearArchivoMemoria(archivoAbierto);




}

void crearArchivo2(char* direccionDisco){



}

void EscribirOModificar(){

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

int buscarPrimerBloqueVacio(int* tablaDeAsignaciones){
	int i;
	while(tablaDeAsignaciones[i]!=-1){
		i++;
		}
	return i;
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

