#!/bin/sh
DEPLOY_FOLDER="/home/utnso/tp-2016-2c-SegmentationFault"

echo Instalando todas las dependencias
#CREO DIRECTORIO (para dependecias) Y ENTRO
mkdir /libraries
cd /libraries
#INSTALAR SO-COMMONS
git clone https://github.com/sisoputnfrba/so-commons-library
cd /so-commons-library
make
sudo make install
echo 

#INSTALAR NCURSES
cd
sudo apt-get install libncurses5-dev
echo 

#INSTALAR NIVEL-GUI
cd $DEPLOY_FOLDER/libraries
git clone https://github.com/sisoputnfrba/so-nivel-gui-library
cd /so-nivel-gui-library
make && make install
echo 

#INSTALAR PKMN-BATTLE
cd $DEPLOY_FOLDER/libraries
git clone https://github.com/sisoputnfrba/so-pkmn-utils
cd /so-pkmn-utils/src
make all
sudo make install
echo 
echo Todas las dependencias instaladas correctamente

#COMPILAR TODOS LOS PROCESOS
dir=`pwd`
for x in PokedexServer PokedexClient Mapa Entrenador Conexiones
do
	cd ${x}/Debug/
	make clean
	make all
	cd ${dir}
done 
#Si no funciona probar con $DEPLOY_FOLDER en lugar de "dir"

echo Deploy finalizado correctamente
echo Si no funciona:
echo 1)Revisar que exista el archivo libpkmn-battle.so dentro del directorio build/
echo 2)Ir al directorio testnivel: cd nivel-gui-test y ejecutar el codigo de ejemplo: ./nivel
