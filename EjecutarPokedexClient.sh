#!/bin/sh
DEPLOY_FOLDER="/home/utnso/git/tp-2016-2c-SegmentationFault"

#CREO DIRECTORIO DONDE VA A ESTAR EL FILE SYSTEM
cd
rm -rf POKEDEX
mkdir POKEDEX
PATH_POKEDEX="/home/utnso/POKEDEX"

#EJECUTAR POKEDEX CLIENT
cd /home/utnso/git/tp-2016-2c-SegmentationFault/PokedexClient/Debug
./PokedexClient $PATH_POKEDEX -f