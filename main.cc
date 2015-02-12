/*
Dibuja una colección de figuritas que rotan ciclicamente por la pantalla. Sus direcciones de movimiento se controlan con las flechas del teclado.
Inicialmente caen en picado.
Si una figura sale por un extremo, aparece por el opuesto en el siguiente frame.
Si se hace click sobre una figura, ésta incrementa ligeramente su tamaño y aumenta su peso, moviéndose más rápido en cada frame.
Los objetos al caer arrastran a los que tocan al caer.
(Opcional) Con una tecla los objetos rotan sobre su centro.
Ventana de 800x600
Un total de 50 figuras: 35 cuadrados de 20x20 y 15 triángulos de 20x10 

*/

#include <ESAT/window.h>
#include <ESAT/draw.h>
#include <ESAT/sprite.h>
#include <ESAT/input.h>

#include <Windows.h>

#include <iosfwd>
#include <string>
#include <time.h>

//type=0 -> cuadrado. type=1 -> triángulo
typedef struct {
	int num_vertices;
	int vertices[4][2];
	int rgb[4];
	int peso;
} tFigura;


int win_width = 800, win_height = 600;


/* Calcula un número aleatorio entre 0 y <limit> */
int random(int limit) {
	return (rand() % limit);
}


/* Dibuja un cuadrado con su esquina superior izquierda en las coordenadas indicadas */
void printFigure (tFigura figura) {
	int i, j;
	float pathPoints[10];

	for (i = 0, j = 0; j < figura.num_vertices; i = i + 2, j++) {
		pathPoints[i] = figura.vertices[j][0];
		pathPoints[i+1] = figura.vertices[j][1];
	}

	pathPoints[i] = figura.vertices[0][0];
	pathPoints[i + 1] = figura.vertices[0][1];

	/*Color rgb interior del polígono*/
	ESAT::DrawSetStrokeColor(figura.rgb[0], figura.rgb[1], figura.rgb[2], figura.rgb[3]);
	ESAT::DrawSetFillColor(0, 0, 0, 0);
	
	/*Pinta la misma figura rellena. El último parámetro determina si se muestra el borde*/
	ESAT::DrawSolidPath(pathPoints, figura.num_vertices+1, true);
}


/* Crea un rectángulo con posición aleatoria y de las dimensiones indicadas */
tFigura createRandomRectangle(int width, int height) {
	int i;
	tFigura rect;
	int posx, posy;

	rect.num_vertices = 4;
	rect.peso = 1;

	//posiciones iniciales del rectángulo
	posx = random(win_width);
	posy = random(win_height);

	for (i = 0; i < 3; i++) {
		rect.rgb[i] = random(255);
	}
	rect.rgb[3] = 255;

	rect.vertices[0][0] = posx; rect.vertices[0][1] = posy;
	rect.vertices[1][0] = posx + width; rect.vertices[1][1] = posy;
	rect.vertices[2][0] = posx + width; rect.vertices[2][1] = posy + height;
	rect.vertices[3][0] = posx; rect.vertices[3][1] = posy + height;

	return rect;
}


void createFigures(tFigura figuras[50]) {
	int i;
	int width, height;
	tFigura figura;

	for (i = 0; i < 50; i++) {
		//comprobamos que cada figura sea de al menos 10px de lado
		do {
			width = random(80);
			height = random(80);
		} while (width < 10 || height < 10);

		figuras[i] = createRandomRectangle(width, height);
	}
}


void printFigures(tFigura figuras[50]) {
	int i;

	for (i = 0; i < 50; i++) {
		printFigure(figuras[i]);
	}
}


void recalculateCoordinates(tFigura (*figura), int newx, int newy) {
	int i;
	// Ésto guardará el conjunto de vectores de la figura, asumiendo que la figura más compleja será un decaedro
	int vectores[10][2];

	//Hallamos los vectores
	for (i = 0; i < (*figura).num_vertices-1; i++) {
		vectores[i][0] = (*figura).vertices[i + 1][0] - (*figura).vertices[i][0];
		vectores[i][1] = (*figura).vertices[i + 1][1] - (*figura).vertices[i][1];
	}

	//El primer vértice la figura estará en las coordenadas indicadas como parámetros
	(*figura).vertices[0][0] = newx;
	(*figura).vertices[0][1] = newy;
	//Sumamos los vectores a los puntos actuales de la figura, tomando como nueva posición inicial los parámetros
	for (i = 1; i < (*figura).num_vertices; i++) {
		(*figura).vertices[i][0] = (*figura).vertices[i - 1][0] + vectores[i-1][0];
		(*figura).vertices[i][1] = (*figura).vertices[i - 1][1] + vectores[i-1][1];
	}
}


void moveFigures (tFigura figuras[50]) {
	int i, j;
	//posiciones de la esquina superior izquierda
	int posx, posy;
	tFigura figura;
	//margen invisible en el exterior de la pantalla
	int margin = 50;

	for (i = 0; i < 50; i++) {
		figura = figuras[i];

		posx = figura.vertices[0][0];
		posy = figura.vertices[0][1];

		for (j = 0; j < figura.num_vertices; j++) {
			figura.vertices[j][1] += figura.peso;
		
			//Detectamos la pulsación de las teclas de dirección laterales y movemos las figuras según corresponda
			if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Left)) {
				figura.vertices[j][0] -= 5;
			}
			else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Right)) {
				figura.vertices[j][0] += 5;
			}
		}

		//Si se sale por la parte inferior de la pantalla, recalculamos sus coordenadas para colocarla en la parte superior de la misma
		if (posy > win_height) {
			recalculateCoordinates(&figura, posx, -margin);
		}
		if (posx > win_width) {
			recalculateCoordinates(&figura, 0-margin, posy);
		}
		else if (posx < -margin) {
			recalculateCoordinates(&figura, win_width + margin, posy);
		}

		figuras[i] = figura;
	}
}


int ESAT::main(int argc, char **argv) {

	tFigura figuras[50];

	srand(time(NULL));

	createFigures(figuras);

	ESAT::WindowInit(win_width, win_height);

	while (ESAT::WindowIsOpened() && !ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)){

		printFigures(figuras);

		moveFigures(figuras);

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}


	ESAT::WindowDestroy();
	return 0;
}