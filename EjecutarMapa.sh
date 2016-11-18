#!/bin/sh
#EJECUTAR MAPA

DEPLOY_FOLDER="/home/utnso/git/tp-2016-2c-SegmentationFault"
PATHPOKEDEX="/home/utnso/git/tp-2016-2c-SegmentationFault/Recursos/PokedexBase"

if [ "$#" -ne 1 ] || [ ! $1 ] ; then
	echo "Uso: $0 [Nombre-Mapa]"
	echo 
	exit 33
fi

NOMBRE_MAPA=$1

cd $DEPLOY_FOLDER/Mapa/Debug
./Mapa $NOMBRE_MAPA $PATHPOKEDEX

