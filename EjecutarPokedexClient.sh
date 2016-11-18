#!/bin/sh
DEPLOY_FOLDER="/home/utnso/tp-2016-2c-SegmentationFault"

#CREO DIRECTORIO DONDE VA A ESTAR EL FILE SYSTEM
rm -rf $DEPLOY_FOLDER/POKEDEX
mkdir -p $DEPLOY_FOLDER/POKEDEX

#EJECUTAR POKEDEX CLIENT
cd /home/utnso/tp-2016-2c-SegmentationFault/PokedexClient/Debug
./PokedexClient $DEPLOY_FOLDER/POKEDEX -f
