/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {
	seniales();
	system("clear");
	time_t comienzo;
	comienzo = time(NULL);
	// Verifica que se haya pasado al menos 1 parametro, sino falla
	assert(("ERROR - No se pasaron argumentos", argc > 1));

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0) {
			entrenador.nombre = argv[i + 1];
			printf("Nombre Entrenador: '%s'\n", entrenador.nombre);
		}
		if (i == 1) {
			entrenador.rutaPokedex = argv[i + 1];
			printf("Ruta Pokedex: '%s'\n", entrenador.rutaPokedex);
		}
	}

	assert(("ERROR - No se paso el nombre del entrenador como argumento", entrenador.nombre != NULL));
	//assert(("ERROR - No se paso la ruta del pokedex como argumento", entrenador.rutaPokedex != NULL));

	entrenador.rutaPokedex = "/home/utnso/PokedexBase";

	//Creo el archivo de Log
	char* logFile = "/home/utnso/git/tp-2016-2c-SegmentationFault/Entrenador/log";
	logFile = string_from_format("%s%s",logFile, entrenador.nombre);
	logEntrenador = log_create(logFile, entrenador.nombre, 1, LOG_LEVEL_TRACE);

	entrenador.hojaDeViaje = list_create();

	//CONFIGURACION DEL ENTRENADOR
	getMetadataEntrenador();
	crearListaPokemones();

	//TODO PROBANDO ENTRENADOR CON POKEDEX LEVANTADO (PARAMETRO DE FUSE: /home/utnso/FUSE/)
	//pruebaCrearYEscribir();
	//pruebaLeer();
	//pruebaBorrar();

	//pruebaOpenDirYReadDir();
	//borrarArchivosEnDirDeBill();
	//borrarMedallas();

	interactuarConMapas();

	time_t final;
	final = time( NULL);
	double timer = difftime(final, comienzo);
	printf("Tiempo Total que tomó toda la aventura: %.2f s\n", timer);
	printf("Cantidad de DeadLocks involucrado: %d\n", cantDeadLocks);
	printf("Tiempo que estuvo bloqueado en las PokeNests: %.2f s\n", tiempoBloqueadoEnPokeNests);
	printf("Cantidad de muertes: %d\n", cantMuertes);

	return EXIT_SUCCESS;
}

void pruebaCrearYEscribir(){

	char* textoArch = "Soy PokPrueba\nEstoy adentro de /home/utnso/FUSE/\nEn el disco challenge.bin\0";
	int textoLen = strlen(textoArch) + 1;

	char* dirPokedex= "/home/utnso/FUSE/PokPrueba.txt\0";

	FILE *archivo = NULL;
	archivo = fopen(dirPokedex, "w");
	fwrite(textoArch, sizeof(char), textoLen, archivo);
	fclose(archivo);

/*
	char* textoArch2 = "Soy PokPrueba2\nEstoy adentro de /home/utnso/FUSE2/\nEn el disco challenge.bin\0";
	int textoLen2 = strlen(textoArch2) + 1;
	char* dirPokedex2= "/home/utnso/FUSE2/PokPrueba2.txt\0";
	FILE *archivo2 = NULL;
	archivo2 = fopen(dirPokedex2, "w");
	fwrite(textoArch2, sizeof(char), textoLen2, archivo2);
	fclose(archivo2);
*/

}

void pruebaBorrar(){
	char* dirPokedex= "/home/utnso/FUSE/PokPrueba.txt\0";

	FILE *archivo = NULL;
	archivo = fopen(dirPokedex, "r");
	if (archivo != NULL) {
		fclose(archivo);
		if (remove(dirPokedex) == 0) printf("PokPrueba.txt Borrado\n");
		else printf("No se pudo borrar archivo PokPrueba.txt\n");
	}
	else printf("Archivo PokPrueba.txt no encontrado\n");
}

void pruebaLeer(){
	int tamanio = 0;
	char* dirPokedex= "/home/utnso/FUSE/PokPrueba.txt\0";
	char* textoArch = leerArchivoYGuardarEnCadena(&tamanio, dirPokedex);
	printf("texto leido de PokPrueba.txt : %s\n",textoArch);
}

void pruebaOpenDirYReadDir(){
	char* fuseDir = string_from_format("/home/utnso/FUSE/Pokemons\0");
	int cont = 0;
	int bytes = 0;
	struct stat estru;
	DIR* dir;

	dir = opendir(fuseDir);
	struct dirent* directorio = NULL;
	while ((directorio = readdir(dir)) != NULL) {
		char* nombrePokemon = directorio->d_name;
		stat(directorio->d_name, &estru);
		if (strcmp(nombrePokemon, ".") == 1 && strcmp(nombrePokemon, "..") == 1) {
			cont++;
		}
		bytes = bytes + estru.st_size;
	}
	printf("cantidad de archivos: %d",cont);
}

void crearListaPokemones(){
	int cantMapas = list_size(entrenador.hojaDeViaje);
	pokemonesCapturados = inicializar(cantMapas * sizeof(char*));
	contextoPokemons = inicializar(cantMapas * sizeof(char*));
	int m;
	for (m = 0; m < cantMapas; m++) {
		pokemonesCapturados[m] = inicializar(sizeof(t_list*));
		pokemonesCapturados[m] = list_create();
		contextoPokemons[m] = inicializar(sizeof(t_list*));
		contextoPokemons[m] = list_create();
	}
}

void *inicializar(int tamanio) {
	int i;
	void * retorno = malloc(tamanio);
	for (i = 0; i < tamanio; i++) {
		((char*) retorno)[i] = 0;
	}
	return retorno;
}

void procesarRecibir(){
	char* mensaje="holaE";
	char* respuesta=malloc(6);
	fflush(stdin);
	recibir(&socketMapa,respuesta,6);

	log_info(logEntrenador,"Mensaje recibido del mapa: %s ", respuesta);
	enviar(&socketMapa, mensaje, 6);
	free(respuesta);
	//close(socketDeMapa);
}

//Funcion que levanta los datos del entrenador
void getMetadataEntrenador() {

	t_config* configEntrenador = malloc(sizeof(t_config));
	configEntrenador->path = string_from_format("%s/Entrenadores/%s/metadata", entrenador.rutaPokedex, entrenador.nombre);
	configEntrenador = config_create(configEntrenador->path);

	entrenador.nombre = config_get_string_value(configEntrenador, "nombre");
	char* simbolo = config_get_string_value(configEntrenador, "simbolo");
	memcpy(&entrenador.simbolo, simbolo, sizeof(entrenador.simbolo));
	entrenador.cantVidas = config_get_int_value(configEntrenador, "vidas");
	char** hojaDeViaje = config_get_array_value(configEntrenador, "hojaDeViaje");
	//inicializo la posicion en 1;1
	entrenador.posicion[0] = 1;
	entrenador.posicion[1] = 1;
	entrenador.mapaActual = 0;
	entrenador.objetivoActual = 0;

	printf("ENTRENADOR - nombre: '%s'. simbolo: '%c'. cantVidas: '%d' \n", entrenador.nombre, entrenador.simbolo, entrenador.cantVidas);
	pokemonMasFuerte.level = -1;

	int i = 0;
	while (hojaDeViaje[i] != NULL) {
		t_mapa* mapa = malloc(sizeof(t_mapa));
		mapa->nombreMapa = hojaDeViaje[i];

		printf("Mapa a recorrer: '%s' con los sig. objetivos: ",mapa->nombreMapa);

		char* strConcat = string_new();
		string_append(&strConcat, "obj[");
		string_append(&strConcat, mapa->nombreMapa);
		string_append(&strConcat, "]");

		//entrenador->mapa->objetivos=config_get_array_value(configEntrenador,"obj[PuebloPaleta]");

		mapa->objetivos = config_get_array_value(configEntrenador, strConcat);

		//esto valida que no haya 2 pokemones seguidos del mismo tipo
		int k = 0;
		int objetivosLen = (int) strlen((char*) mapa->objetivos) / sizeof(char*);
		while(k < objetivosLen){
			if (k+1 != objetivosLen){
				if (strcmp(mapa->objetivos[k], mapa->objetivos[k+1]) == 0){
					log_error(logEntrenador,"\nERROR: 2 POKEMONS DEL MISMO TIPO DE FORMA CONSECUTIVA \n");
					log_error(logEntrenador,"Pokemon pos %d: %s, Pokemon pos %d: %s", k,
							mapa->objetivos[k], k + 1, mapa->objetivos[k + 1]);
					exit(-1);
				}
			}
			k++;
		}

		imprimirObjetivos(mapa);

		t_config* configMapa = malloc(sizeof(t_config));

		configMapa->path = string_from_format("%s/Mapas/%s/metadata",entrenador.rutaPokedex, mapa->nombreMapa);
		configMapa = config_create(configMapa->path);
		mapa->ip = config_get_string_value(configMapa, "IP");
		mapa->puerto = config_get_int_value(configMapa,"Puerto");
		printf("MAPA %s - IP: %s. PUERTO: %d \n", mapa->nombreMapa, mapa->ip, mapa->puerto);

		list_add(entrenador.hojaDeViaje, (void*) mapa);

		i++;
	}

	printf("La cantidad de mapas a recorrer es: %d \n", entrenador.hojaDeViaje->elements_count);

	recorrerEPrintearLista(entrenador.hojaDeViaje);

}

void getObjetivos(){
	t_config* configEntrenador = malloc(sizeof(t_config));
	configEntrenador->path = string_from_format("%s/Entrenadores/%s/metadata", entrenador.rutaPokedex, entrenador.nombre);
	configEntrenador = config_create(configEntrenador->path);
	int i = 0;
	int cantMapas = list_size(entrenador.hojaDeViaje);
	while(i < cantMapas){
		t_mapa* mapa = (t_mapa*) list_get(entrenador.hojaDeViaje, i);
		printf("Mapa a recorrer: '%s' con los sig. objetivos: ",mapa->nombreMapa);

		char* strConcat = string_new();
		string_append(&strConcat, "obj[");
		string_append(&strConcat, mapa->nombreMapa);
		string_append(&strConcat, "]");

		mapa->objetivos = config_get_array_value(configEntrenador, strConcat);

		imprimirObjetivos(mapa);
		i++;
	}
}

void imprimirObjetivos(t_mapa* mapa){
	int j = 0;
	while (mapa->objetivos[j] != NULL) {

		if (mapa->objetivos[j + 1] != NULL) {
			printf("%s, ", mapa->objetivos[j]);

		} else {
			printf("%s \n", mapa->objetivos[j]);
		}

		j++;
	}
}

void recorrerEPrintearLista(t_list* unaLista){
	int i;
	t_mapa* unMapa;
	for (i = 0; i < unaLista->elements_count; i++) {
		unMapa = (t_mapa*) list_get(unaLista, i);
		printf("MAPA %s - IP: %s. PUERTO: %i \n",unMapa->nombreMapa,unMapa->ip,unMapa->puerto);
	}
}

void interactuarConMapas(){
	int cantMapas = list_size(entrenador.hojaDeViaje);
	while(entrenador.mapaActual < cantMapas){ //todo recorrer mapas
		t_mapa* mapa;
		mapa = (t_mapa*) list_get(entrenador.hojaDeViaje,entrenador.mapaActual);

		//socketMapa=conectarseA("10.0.2.15",1982);
		socketMapa = conectarseA(mapa->ip, mapa->puerto);
		if (socketMapa >0){
			printf(": me conecte al mapa %s\n",mapa->nombreMapa);
		}else{
			log_error(logEntrenador,"No me pude conectar. ");
			return;
		}

		enviarInfoAlMapa();	//Envio al mapa los datos de entrenador y el objetivo
		recibir(&socketMapa, &esMiTurno, sizeof(bool));

		//Solicito la posicion de la pokenest de mi proximo objetivo (se le notifica cual es el objetivo)
		//Se recibe la pos de la pokenest pasando por referencia y ya trae la pos
		int i = 0;

		solicitarUbicacionPokenest(&posObjX, &posObjY, i);
		recibir(&socketMapa, &esMiTurno, sizeof(bool));

		volverAlMismoMapa = false;
		cumpliObjetivos = false;
		abandonar = -1;

		while(esMiTurno){

			//Confirmo no haber llegado a la pokenest
			if(entrenador.posicion[0]==posObjX && entrenador.posicion[1]==posObjY){
				//Solicito atrapar al pokemon ¡Llegue a la pokenest!
				char pokemon;
				memcpy(&pokemon, mapa->objetivos[i], sizeof(char));
				flagAtrapar = true;
				atraparUnPokemon(pokemon);//tener en cuenta que ya se habia enviado el primer objetivo
				i++; //se debe ir actualizando el objetivo
				flagAtrapar = false;
			}else{
				//No llegue pido para seguir avanzando
				avanzarHastaPokenest(posObjX, posObjY);
			}
			if (volverAlMismoMapa || abandonar != -1 || cumpliObjetivos) break;
			recibir(&socketMapa, &esMiTurno, sizeof(bool));
			if (cumpliObjetivos) break;

		}

		liberarRecursosCapturados();

		if (!volverAlMismoMapa) entrenador.mapaActual++;
		if (abandonar == 0) entrenador.mapaActual = 0;
		if (abandonar == 1) return;
	}
}

void enviarInfoAlMapa(){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;
	mensaje.objetivoActual = 0;

	t_mapa* mapa;
	mapa = list_get(entrenador.hojaDeViaje,entrenador.mapaActual);
	if(mapa->objetivos[0]!=NULL)memcpy(&mensaje.objetivoActual, mapa->objetivos[0], sizeof(char));//todo verificar de enviar su objetivo
	entrenador.objetivoActual = mensaje.objetivoActual;
	mensaje.operacion = -1;//no es necesario pero se inicializa
	int nombreLen = strlen(mensaje.nombreEntrenador) + 1;

	int payloadSize= sizeof(mensaje.id) + sizeof(mensaje.objetivoActual) + sizeof(mensaje.operacion)
			+ sizeof(nombreLen) + nombreLen;
	int bufferSize= sizeof(bufferSize) + payloadSize;

	// Serializar y enviar al ENTRENADOR
	char* bufferAEnviar = malloc(bufferSize);
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar,payloadSize);
	enviar(&socketMapa, bufferAEnviar, bufferSize);

	free(bufferAEnviar);
	log_info(logEntrenador,"envie mis datos iniciales al Mapa");
}

void solicitarUbicacionPokenest(int* posx, int* posy, int index){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 1;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;

	//t_mapa* mapa;
	//mapa = list_get(entrenador.hojaDeViaje,entrenador.mapaActual);
	//memcpy(&mensaje.objetivoActual, mapa->objetivos[index], sizeof(mensaje.objetivoActual));

	memcpy(&mensaje.objetivoActual, &entrenador.objetivoActual, sizeof(mensaje.objetivoActual));

	int nombreLen = strlen(mensaje.nombreEntrenador) + 1;

	int payloadSize= sizeof(mensaje.id) + sizeof(mensaje.objetivoActual) + sizeof(mensaje.operacion)
			+ sizeof(nombreLen) + nombreLen;
	int bufferSize= sizeof(bufferSize) + payloadSize;

	// Serializar y enviar al ENTRENADOR
	char* bufferAEnviar = malloc(bufferSize);
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar,payloadSize);
	enviar(&socketMapa, bufferAEnviar, bufferSize);

	free(bufferAEnviar);

	int posicionX;
	int posicionY;
	int bytesRecibidosX = recibir(&socketMapa, &posicionX,sizeof(int));
	int bytesRecibidosY = recibir(&socketMapa, &posicionY,sizeof(int));

	if(bytesRecibidosX > 0 && bytesRecibidosY > 0){
		*posx = posicionX;
		*posy = posicionY;
		log_info(logEntrenador,"posicionX: %d. posicionY: %d.",posicionX,posicionY);
	}else{
		log_error(logEntrenador,"Se recibio un tamanio distinto al esperado ");
	}
}

void avanzarHastaPokenest(int posicionXPokenest, int posicionYPokenest){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 2;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;
	mensaje.objetivoActual = entrenador.objetivoActual;

	int nombreLen = strlen(mensaje.nombreEntrenador) + 1;
	int payloadSize= sizeof(mensaje.id) + sizeof(mensaje.objetivoActual) + sizeof(mensaje.operacion)
			+ sizeof(nombreLen) + nombreLen;
	int bufferSize= sizeof(bufferSize) + payloadSize;

	// Serializar y enviar al ENTRENADOR
	char* bufferAEnviar = malloc(bufferSize);
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar,payloadSize);
	enviar(&socketMapa, bufferAEnviar, bufferSize);

	free(bufferAEnviar);

	avanzarPosicion(&entrenador.posicion[0], &entrenador.posicion[1],
			posicionXPokenest, posicionYPokenest);
	enviar(&socketMapa, &entrenador.posicion[0], sizeof(int));
	enviar(&socketMapa, &entrenador.posicion[1], sizeof(int));

}

void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY){
	int posicionX = *actualX;
	int posicionY = *actualY;
	if (!alternateFlag) { //si esta en false entra
		if (posicionX > destinoX) {
			posicionX--;
		} else if (posicionX < destinoX) {
			posicionX++;
		}
		if (posicionY != destinoY) alternateFlag = true;

	} else if (alternateFlag) { //si esta en true entra
		if (posicionY > destinoY) {
			posicionY--;
		} else if (posicionY < destinoY) {
			posicionY++;
		}
		if (posicionX != destinoX) alternateFlag = false;
	}
	*actualX = posicionX;
	*actualY = posicionY;
}

void atraparUnPokemon(char pokemon){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 3;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;
	mensaje.objetivoActual = pokemon;//Envio el pokemon que necesito el mapa me confirma la resolucion

	int nombreLen = strlen(mensaje.nombreEntrenador) + 1;
	int payloadSize= sizeof(mensaje.id) + sizeof(mensaje.objetivoActual) + sizeof(mensaje.operacion)
			+ sizeof(nombreLen) + nombreLen;
	int bufferSize= sizeof(bufferSize) + payloadSize;

	// Serializar y enviar al ENTRENADOR
	char* bufferAEnviar = malloc(bufferSize);
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar,payloadSize);
	enviar(&socketMapa, bufferAEnviar, bufferSize);

	free(bufferAEnviar);
	int resolucionCaptura;
	int resolucionDeBatalla;
	bool repetir = true;//repite cuando gana la batalla
	while (repetir) {
		repetir = false;
		double tiempoBloqueado;
		recibir(&socketMapa, &tiempoBloqueado, sizeof(double));
		tiempoBloqueadoEnPokeNests += tiempoBloqueado;
		recibir(&socketMapa, &resolucionCaptura, sizeof(int));
		if (resolucionCaptura == 0) {  // 0 es para capturar
			log_info(logEntrenador,"se captura el pokemon: %c ", pokemon);
			capturarPokemon();
			chequearObjetivos(pokemon);
		} else if (resolucionCaptura == 1) {  // 1 es por batalla y se envia el pokemon mas fuerte
			log_info(logEntrenador,"se envia el pokemon mas fuerte: %s ", pokemonMasFuerte.species);
			enviarPokemon(socketMapa, &pokemonMasFuerte);
			bool continuar = true;
			while(continuar){
				continuar = false;
				recibir(&socketMapa, &resolucionDeBatalla, sizeof(int));
				if (resolucionDeBatalla == 1){
					log_info(logEntrenador,"Mi pokemon mas fuerte gano la batalla. ");
					repetir = true;
					cantDeadLocks ++;
				}else if (resolucionDeBatalla == 0){
					log_info(logEntrenador,"Mi pokemon mas fuerte perdio la batalla y fui seleccionado como victima. ");
					muerteDelEntrenador();
				}else{
					log_info(logEntrenador,"Mi pokemon mas fuerte perdio la batalla y debe batallar nuevamente. ");
					continuar = true;
				}
			}
			//imprimirListasPokemones();

			if (!repetir) cantDeadLocks ++;
		} else {
			//chequearVidas(entrenador);
		}
	}
}

void capturarPokemon(){
	int tamanioPokemon = 0;
	recibir(&socketMapa, &tamanioPokemon, sizeof(int));
	t_pokemon* pokemon = malloc(tamanioPokemon - sizeof(int));
	pokemon->species = string_new();
	recibirPokemon(socketMapa, pokemon);
	list_add(pokemonesCapturados[entrenador.mapaActual], (void*) pokemon);

	int tamanioContexto = 0;
	recibir(&socketMapa, &tamanioContexto, sizeof(int));
	t_contextoPokemon* contextoPokemon = malloc(tamanioContexto - sizeof(int));//resto 4 para q no incluya el payloadSize
	contextoPokemon->nombreArchivo = string_new();
	contextoPokemon->pathArchivo = string_new();

	recibirContextoPokemon(socketMapa, contextoPokemon);
	list_add(contextoPokemons[entrenador.mapaActual], (void*) contextoPokemon);

	//todo probar con pokedex levantado
	int tamanioDeArchivo = 0;
	char* textoArch = string_new();
	textoArch = leerArchivoYGuardarEnCadena(&tamanioDeArchivo, contextoPokemon->pathArchivo);
	guardarEnDirdeBill(contextoPokemon->nombreArchivo , tamanioDeArchivo, textoArch);

	if (pokemonMasFuerte.level == -1){
		memcpy(&pokemonMasFuerte, pokemon, sizeof(t_pokemon));
	}else if (pokemonMasFuerte.level != -1 && pokemonMasFuerte.level < pokemon->level) {
			log_info(logEntrenador,"Pokemon mas fuerte anterior: %s - Level %d ",
					pokemonMasFuerte.species,
					pokemonMasFuerte.level);
			memcpy(&pokemonMasFuerte, pokemon, sizeof(t_pokemon));
	}
	log_info(logEntrenador,"Pokemon mas fuerte actual: %s - Level %d ",
			pokemonMasFuerte.species,
			pokemonMasFuerte.level);
	imprimirListasPokemones();
}


void* leerArchivoYGuardarEnCadena(int* tamanioDeArchivo, char* nombreDelArchivo) {
	FILE* archivo = NULL;

	int descriptorArchivo = 0;
	archivo = fopen(nombreDelArchivo, "r+");
	descriptorArchivo = fileno(archivo);
	lseek(descriptorArchivo, 0, SEEK_END);
	*tamanioDeArchivo = ftell(archivo);
	char* textoDeArchivo = malloc(*tamanioDeArchivo);
	lseek(descriptorArchivo, 0, SEEK_SET);
	if (archivo == NULL) {
		log_error(logEntrenador, "Error al abrir el archivo.");
	} else {
		size_t count = 1;
		count = fread(textoDeArchivo, *tamanioDeArchivo, count, archivo);
		memset(textoDeArchivo + *tamanioDeArchivo,'\0',1);
	}
	//fclose(archivo);
	return textoDeArchivo;
}


void guardarEnDirdeBill(char* nombreArchivo, int tamanioDeArchivo, char* textoArch){
	char* archivoEnDirDeBill = string_from_format("%s/Entrenadores/%s/Dir de Bill/%s\0",
			entrenador.rutaPokedex, entrenador.nombre, nombreArchivo);
	FILE *archivo = NULL;
	archivo = fopen(archivoEnDirDeBill, "w+");
	fwrite(textoArch, sizeof(char), tamanioDeArchivo, archivo);

	//free(textoArch);
	fclose(archivo);
}

void borrarArchivosEnDirDeBill(){
	char* dirDeBill = string_from_format("%s/Entrenadores/%s/Dir de Bill/\0",
				entrenador.rutaPokedex, entrenador.nombre);
	int bytes = 0;
	struct stat estru;
	DIR* dir;

	dir = opendir(dirDeBill);
	struct dirent* directorio = NULL;
	while ((directorio = readdir(dir)) != NULL) {
		char* nombrePokemon = directorio->d_name;
		stat(directorio->d_name, &estru);
		if (strcmp(nombrePokemon, ".") == 1 && strcmp(nombrePokemon, "..") == 1) {
			char* pathArchivo = string_new();
			pathArchivo = string_from_format("%s%s", dirDeBill, nombrePokemon);

			FILE *archivo = NULL;
			archivo = fopen(pathArchivo, "r");
			if (archivo != NULL) {
				fclose(archivo);
				if (remove(pathArchivo) == 0) log_info(logEntrenador,"Archivo Borrado: %s", nombrePokemon);
				else log_error(logEntrenador, "No se pudo borrar archivo: %s",nombrePokemon);
			}
			else log_warning(logEntrenador,"Archivo %s no encontrado", nombrePokemon);
		}
		bytes = bytes + estru.st_size;
	}
}

void borrarMedallas(){
	char* dirMedallas = string_from_format("%s/Entrenadores/%s/medallas/\0",
			entrenador.rutaPokedex, entrenador.nombre);
	int bytes = 0;
	struct stat estru;
	DIR* dir;

	dir = opendir(dirMedallas);
	struct dirent* directorio = NULL;
	while ((directorio = readdir(dir)) != NULL) {
		char* medalla = directorio->d_name;
		stat(directorio->d_name, &estru);
		if (strcmp(medalla, ".") == 1 && strcmp(medalla, "..") == 1) {
			char* pathArchivo = string_new();
			pathArchivo = string_from_format("%s%s", dirMedallas, medalla);

			FILE *archivo = NULL;
			archivo = fopen(pathArchivo, "r");
			if (archivo != NULL) {
				fclose(archivo);
				if (remove(pathArchivo) == 0) log_info(logEntrenador,"Archivo Borrado: %s", medalla);
				else log_error(logEntrenador, "No se pudo borrar archivo: %s",medalla);
			}
			else log_warning(logEntrenador,"Archivo %s no encontrado", medalla);
		}
		bytes = bytes + estru.st_size;
	}
}

void imprimirListasPokemones() {
	int i = 0;
	int cantMapas = list_size(entrenador.hojaDeViaje);
	while (i < cantMapas) {
		if (list_size(pokemonesCapturados[i]) > 0) {
			int j = 0;
			t_mapa* mapa = (t_mapa*) list_get(entrenador.hojaDeViaje, i);
			log_info(logEntrenador,"Pokemones del mapa %s:", mapa->nombreMapa);
			while(j < pokemonesCapturados[i]->elements_count){

				t_pokemon* pokemon = (t_pokemon*) list_get(pokemonesCapturados[i], j);

				log_info(logEntrenador,"%s - Level %d", pokemon->species, pokemon->level);
				j++;
			}
		}
		i++;
	}
	if (i == 0)
		log_info(logEntrenador,"No hay pokemones en las listas");

	if (pokemonesCapturados == NULL)
		log_info(logEntrenador, "-----------------");
}

void chequearObjetivos(char pokemon){
	t_mapa* mapaEnElQueEstoy = (t_mapa*) list_get(entrenador.hojaDeViaje, entrenador.mapaActual);

	int i=0;
	char objetivo;
	while(mapaEnElQueEstoy->objetivos[i]!=NULL){
		memcpy(&objetivo, mapaEnElQueEstoy->objetivos[i], sizeof(char));
		if (objetivo == pokemon){
			mapaEnElQueEstoy->objetivos[i] = "NO";//marco que ya no es un objetivo
			entrenador.objetivoActual = 0;
			break;
		}
		i++;
	}
	int cantObjetivos = (int) strlen((char*) mapaEnElQueEstoy->objetivos) /sizeof(char*);
	//log_info(logEntrenador,"cantObjetivos: %d. ", cantObjetivos);

	if(mapaEnElQueEstoy->objetivos[i+1]==NULL){
		copiarMedallaDelMapa(mapaEnElQueEstoy->nombreMapa);//todo copiar medalla a su directorio
		cumpliObjetivos = true;
		if (list_size(entrenador.hojaDeViaje) - 1 == entrenador.mapaActual) {
			log_info(logEntrenador,"Eres un maestro pokemon completaste la aventura.");
		}else{
			log_info(logEntrenador,"Complete los objetivos del mapa actual.");
		}

	}else{
		i++;
		if (i < cantObjetivos) {
			recibir(&socketMapa, &esMiTurno, sizeof(bool));
			memcpy(&entrenador.objetivoActual, mapaEnElQueEstoy->objetivos[i], sizeof(char));
			solicitarUbicacionPokenest(&posObjX, &posObjY, i);
		}
	}
}

void copiarMedallaDelMapa(char* nombreDelMapa){
	// pathMedalla: /Mapas/[nombre]/medalla-[nombre].jpg
	char* nombreMedalla = string_from_format("medalla-%s.jpg\0", nombreDelMapa);
	char* dirMedallaMapa = string_from_format("%s/Mapas/%s/%s\0",
			entrenador.rutaPokedex, nombreDelMapa, nombreMedalla);
	char* dirMedallaEntrenador = string_from_format("%s/Entrenadores/%s/medallas/%s\0",
				entrenador.rutaPokedex, entrenador.nombre,nombreMedalla);
	int tamanio = 0;
	char* cadena = leerArchivoYGuardarEnCadena(&tamanio, dirMedallaMapa);
	FILE *archivo = NULL;
	archivo = fopen(dirMedallaEntrenador, "w+");
	fwrite(cadena, sizeof(char), tamanio, archivo);
	fclose(archivo);
}

void verificarTurno(){
	int mensajeLen = 0;
	recibir(&socketMapa, &mensajeLen, sizeof(int));
	char* mensajeTurno = malloc(mensajeLen);
	recibir(&socketMapa, mensajeTurno, mensajeLen);
	esMiTurno = strcmp(mensajeTurno, "turno concedido") == 0;
	if (esMiTurno) {
		free(mensajeTurno);
		return;
	}
}

void agregarVida(){
		entrenador.cantVidas ++;
		log_info(logEntrenador,"+1 Vida ");
}

void controladorDeSeniales(int signo) {
	switch (signo) {
	case SIGUSR1: {
		agregarVida();
		log_info(logEntrenador,"La cantidad de vidas del jugador %s es: %i ",entrenador.nombre, entrenador.cantVidas);
		break;
	}
	case SIGINT:
	case SIGKILL:
	{
		log_info(logEntrenador,"\n Abandono el juego \n");
		bool senial = true;
		if(flagAtrapar) send(socketMapa, &senial, sizeof(bool), MSG_DONTWAIT);
		cumpliObjetivos = true;//para que envie la operacion
		liberarRecursosCapturados();
		cumpliObjetivos = false;
		exit(1);
		break;
	}
	case SIGTERM: {
		muerteDelEntrenador();
		break;
	}
	default:
		printf("Signal distinta a la esperada, intentar nuevamente\n");
	}
}

void liberarRecursosCapturados(){
	log_info(logEntrenador,"Libero los pokemons capturados");
	if(cumpliObjetivos){
		t_MensajeEntrenador_Mapa mensaje;

		mensaje.operacion = 5;
		mensaje.nombreEntrenador = entrenador.nombre;
		mensaje.id = entrenador.simbolo;

		int nombreLen = strlen(mensaje.nombreEntrenador) + 1;
		int payloadSize= sizeof(mensaje.id) + sizeof(mensaje.objetivoActual) + sizeof(mensaje.operacion)
				+ sizeof(nombreLen) + nombreLen;
		int bufferSize= sizeof(bufferSize) + payloadSize;

		// Serializar y enviar al ENTRENADOR
		char* bufferAEnviar = malloc(bufferSize);
		serializarEntrenador_Mapa(&mensaje, bufferAEnviar,payloadSize);
		enviar(&socketMapa, bufferAEnviar, bufferSize);

		free(bufferAEnviar);
	}
	imprimirListasPokemones();

	int i = 0;
	int m = entrenador.mapaActual;
	int cantPokemones = list_size(pokemonesCapturados[m]);
	enviar(&socketMapa, &cantPokemones, sizeof(int));
	while (i < cantPokemones){
		t_pokemon* pokemonCapturado = (t_pokemon*) list_get(pokemonesCapturados[m], i);
		t_contextoPokemon* contexto = (t_contextoPokemon*) list_get(contextoPokemons[m], i);

		enviarPokemon(socketMapa, pokemonCapturado);
		enviarContextoPokemon(socketMapa, contexto);

		//log_info(logEntrenador,"ENVIO - nom: %s. texto: %s\n",contexto->nombreArchivo, contexto->textoArch);
		if(!cumpliObjetivos && abandonar == 0){
			if(strcmp(pokemonCapturado->species, pokemonMasFuerte.species) == 0){
				actualizarPokemonMasFuerte(pokemonCapturado);
			}
			//list_remove(pokemonesCapturados[m], 0);
			//free(pokemonCapturado);
			//t_contextoPokemon* contextoPokemon= (t_contextoPokemon*)list_remove(contextoPokemons[m], 0);
			//free(contextoPokemon);
		}
		i++;
	}
	shutdown(socketMapa, 1);
	entrenador.posicion[0] = 1;
	entrenador.posicion[1] = 1;
	//imprimirListasPokemones();
	list_clean_and_destroy_elements(pokemonesCapturados[m], (void*) destruirPokemon);
	list_clean_and_destroy_elements(contextoPokemons[m], (void*) destruirContexto);

}

void actualizarPokemonMasFuerte(t_pokemon* pokemonALiberar) {
	pokemonMasFuerte.level = 0;
	pokemonMasFuerte.species = string_new();
	pokemonMasFuerte.type = NO_TYPE;
	pokemonMasFuerte.second_type = NO_TYPE;

	int cantMapas = list_size(entrenador.hojaDeViaje);
	int m = 0;
	while (m < cantMapas) {
		int i = 0;
		int cantPokemones = list_size(pokemonesCapturados[m]);
		while (i < cantPokemones) {
			t_pokemon* pokemonCapturado = (t_pokemon*) list_get(pokemonesCapturados[m], i);
			if(pokemonCapturado->level > pokemonMasFuerte.level && pokemonCapturado != pokemonALiberar){
				strcpy(pokemonMasFuerte.species, pokemonCapturado->species);
				memcpy(&pokemonMasFuerte, pokemonCapturado, sizeof(t_pokemon));
				log_info(logEntrenador,"nuevo pokemon mas fuerte: %s. Level %d", pokemonMasFuerte.species, pokemonMasFuerte.level);

			}
			i++;
		}
		m++;
	}
}

void destruirPokemon(t_pokemon* unPokemon){
	free(unPokemon->species);
	free(unPokemon);
}

void destruirContexto(t_contextoPokemon* contexto){
	free(contexto->nombreArchivo);
	free(contexto->pathArchivo);
	free(contexto);
}

void seniales(){

	signal(SIGUSR1, controladorDeSeniales);
	signal(SIGKILL, controladorDeSeniales);
	signal(SIGINT, controladorDeSeniales);
	signal(SIGTERM, controladorDeSeniales);
}

void muerteDelEntrenador(){
	entrenador.cantVidas--;
	cantMuertes ++;
	borrarArchivosEnDirDeBill();
	if(entrenador.cantVidas==0){
		log_info(logEntrenador,"Me quede sin vidas y la cantidad de reintentos fue: %d", reintentos);
		//log_info(logEntrenador,"¿Desea reiniciar el juego? Y/N ");
		printf("¿Desea reiniciar el juego? Y/N \n");
		char respuesta = getchar();
		if (respuesta == 'Y'){
			reintentos++;
			entrenador.mapaActual = 0;
			getObjetivos();
			borrarMedallas();
			abandonar = 0;//con esto se liberan los pokemons
		}else{
			abandonar = 1;
		}
	}else{
		if(entrenador.cantVidas==1) log_info(logEntrenador,"Perdi una vida, me queda: %i vida ",entrenador.cantVidas);
		log_info(logEntrenador,"Perdi una vida, me quedan: %i vidas",entrenador.cantVidas);
		getObjetivos();
		volverAlMismoMapa = true;
	}
}
