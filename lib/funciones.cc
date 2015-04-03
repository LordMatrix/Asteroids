/*
Librería de funciones generales

Marcos Vázquez Rey
ESAT 2015
*/

#include <ESAT/window.h>
#include <ESAT/draw.h>
#include <ESAT/sprite.h>
#include <ESAT/input.h>

#include <Windows.h>

#include <iosfwd>
#include <string>
#include <time.h>

/* Calcula un número aleatorio entre 0 y <limit> */
int random(int limit) {
	return (rand() % limit);
}

/* Devuelve aleatoriamente 1 o -1*/
int random_sign() {
	if (random(2) == 1)
		return 1;
	else
		return -1;
}