#include "pokedexServer.h"

int main(){
	int posicionPokenest[2];
	posicionPokenest=solicitoPosicion();
	printf("%i \n",posicionPokenest[0]);
	printf("%i \n",posicionPokenest[1]);
}


int solicitoPosicion(){
	int posicion[2];
	posicion[0]=1;
	posicion[1]=2;
	return posicion;
}
