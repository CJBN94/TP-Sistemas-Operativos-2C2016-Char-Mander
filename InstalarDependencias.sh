#!/bin/sh

echo Instalando todas las dependencias
#CREO DIRECTORIO (para dependecias) Y ENTRO
cd
rm -rf libraries
mkdir libraries
cd libraries
#INSTALAR SO-COMMONS
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make
sudo make install
echo 

#INSTALAR NCURSES
cd
sudo apt-get install libncurses5-dev
echo 

#INSTALAR NIVEL-GUI
cd libraries
git clone https://github.com/sisoputnfrba/so-nivel-gui-library
cd so-nivel-gui-library
make && make install
cd
echo Para probar su correcta instalacion: cd nivel-gui-test y ejecutar ./nivel
echo 

#INSTALAR PKMN-BATTLE
cd libraries
git clone https://github.com/sisoputnfrba/so-pkmn-utils
cd so-pkmn-utils/src
make all
sudo make install
cd build
ls
echo Revisar que exista libpkmn-battle.so
cd
echo 

#CLONAR UTILS Y TESTS
cd libraries
git clone https://github.com/sisoputnfrba/osada-utils
cd
cd libraries
git clone https://github.com/sisoputnfrba/osada-tests
cd
sudo apt-get install openssl
sudo apt-get install libssl-dev
cd libraries
git clone https://github.com/sisoputnfrba/massive-file-creator
cd massive-file-creator
make && make all
cd
cd libraries
git clone https://github.com/sisoputnfrba/so-charmander-pruebas
cd 
echo

echo Todas las dependencias instaladas correctamente
echo 
