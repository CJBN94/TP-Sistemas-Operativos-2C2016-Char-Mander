/*
 * pokedexServer.c
 *
 */

#include "pokedexServer.h"
#include "osada.h"


osada_header* miFileSystem;


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
	tamanioFileSystem=1024;
	miFileSystem->fs_blocks=tamanioFileSystem/OSADA_BLOCK_SIZE;
	miFileSystem->version=1;
	miFileSystem->bitmap_blocks=miFileSystem->fs_blocks/8/OSADA_BLOCK_SIZE;
	miFileSystem->allocations_table_offset=1+miFileSystem->bitmap_blocks+1024;
	int tamanioTablaDeAsignaciones=(miFileSystem->fs_blocks-1-miFileSystem->bitmap_blocks-1024)*4/OSADA_BLOCK_SIZE;
	miFileSystem->data_blocks=miFileSystem->fs_blocks-1-miFileSystem->bitmap_blocks-tamanioTablaDeAsignaciones;
	int tablaDeAsignaciones[miFileSystem->data_blocks];
	osada_file tablaDeArchivos[2048];
	t_bitarray* mapaDeBits=NULL;
	char* mapa=malloc(miFileSystem->bitmap_blocks/8);
	mapaDeBits=bitarray_create(mapa,miFileSystem->bitmap_blocks);

	return EXIT_SUCCESS;

}


