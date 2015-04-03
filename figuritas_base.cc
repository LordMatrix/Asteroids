/*
Dibuja una colección de figuritas que rotan ciclicamente por la pantalla. Sus direcciones de movimiento se controlan con las flechas del dirección.
Inicialmente caen en picado.
Si una figura sale por un extremo, aparece por el opuesto en el siguiente frame.
Si se hace click sobre una figura, ésta incrementa ligeramente su tamaño y aumenta su peso, moviéndose más rápido en cada frame.
Pulsando G las figuras rotan sobre su centro.
Ventana de 800x600
las figuras son polígonos convexos de entre 3 y 10 vértices generados aleatoriamente.

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

#define M_PI 3.14159265358979323846;

typedef struct {
	float x;
	float y;
} tPoint;

typedef struct {
	int num_vertices;
	tPoint vertices[20];
	tPoint center;
	int rgb[4];
	float peso;
} tFigura;

typedef struct {
	tFigura figura;
	float accx, accy;
	float dir;
} tShip;

typedef struct {
	tPoint pos;
	float accx, accy;
} tShot;

int win_width = 800, win_height = 600;


/* Calcula un número aleatorio entre 0 y <limit> */
int random(int limit) {
	return (rand() % limit);
}


/* Dibuja un cuadrado con su esquina superior izquierda en las coordenadas indicadas */
void printFigure(tFigura figura) {
	int i, j;
	float pathPoints[50];

	for (i = 0, j = 0; j < figura.num_vertices; i = i + 2, j++) {
		pathPoints[i] = figura.vertices[j].x;
		pathPoints[i + 1] = figura.vertices[j].y;
	}

	pathPoints[i] = figura.vertices[0].x;
	pathPoints[i + 1] = figura.vertices[0].y;

	/*Color rgb interior del polígono*/
	ESAT::DrawSetStrokeColor(figura.rgb[0], figura.rgb[1], figura.rgb[2], figura.rgb[3]);
	ESAT::DrawSetFillColor(0, 0, 0, 0);

	/*Pinta la misma figura rellena. El último parámetro determina si se muestra el borde*/
	ESAT::DrawSolidPath(pathPoints, figura.num_vertices + 1, true);
}


/* Dibuja un punto en el centro de la figura */
void printFigureCenter(tFigura figura) {
	float cx = figura.center.x;
	float cy = figura.center.y;

	float pathPoints2[20] = {
		cx - 1, cy - 1,
		cx + 1, cy - 1,
		cx + 1, cy + 1,
		cx - 1, cy + 1,
		cx - 1, cy - 1
	};
	ESAT::DrawSolidPath(pathPoints2, 5, true);
}


/* Dibuja todas las figuras contenida en el array pasado como parámetro */
void printFigures(tFigura figuras[50], int num_figuras, int printCenter) {
	int i;

	for (i = 0; i < num_figuras; i++) {
		printFigure(figuras[i]);
		if (printCenter)
			printFigureCenter(figuras[i]);
	}
}


/* Dibuja los disparos */
void printShots(tShot shots[50], int num_shots) {
	int i;

	for (i = 0; i < num_shots; i++) {
		float pathPoints2[20] = {
			shots[i].pos.x, shots[i].pos.y,
			shots[i].pos.x+10, shots[i].pos.y+10,
		};
	}
}


/* Calcula el centroide y devuelve los puntos x e y pertenecientes al centro del polígono */
void getFigureCenter(tFigura figura, float *x, float *y) {

	int i = 0;

	float signedArea = 0.0;
	float x0 = 0.0; // Current vertex X
	float y0 = 0.0; // Current vertex Y
	float x1 = 0.0; // Next vertex X
	float y1 = 0.0; // Next vertex Y
	float a = 0.0;  // Partial signed area

	*x = 0; *y = 0;

	// para cada vértice salvo el último
	for (i = 0; i < figura.num_vertices - 1; ++i)
	{
		x0 = figura.vertices[i].x;
		y0 = figura.vertices[i].y;
		x1 = figura.vertices[i + 1].x;
		y1 = figura.vertices[i + 1].y;
		a = x0*y1 - x1*y0;
		signedArea += a;
		*x += (x0 + x1)*a;
		*y += (y0 + y1)*a;
	}

	// Último vértice
	x0 = figura.vertices[i].x;
	y0 = figura.vertices[i].y;
	x1 = figura.vertices[0].x;
	y1 = figura.vertices[0].y;
	a = x0*y1 - x1*y0;
	signedArea += a;
	*x += (x0 + x1)*a;
	*y += (y0 + y1)*a;

	signedArea *= 0.5;
	*x /= (6 * signedArea);
	*y /= (6 * signedArea);
}


/* Crea una figura aleatoria de dimensiones máximas y número de vértices indicados */
tFigura createRandomFigure(int width, int height, int num_vertices) {
	int i;
	tFigura figure;
	float dx, dy;
	float angle = 360/(num_vertices);
	float rad;
	int max_radius, radius;

	figure.num_vertices = num_vertices;
	figure.peso = (float)(random(10)+1) / 10.0;

	//posiciones iniciales del rectángulo
	figure.center.x = random(win_width);
	figure.center.y = random(win_height);

	for (i = 0; i < 3; i++) {
		figure.rgb[i] = random(255);
	}
	figure.rgb[3] = 255;

	max_radius = sqrt(pow(width, 2) + pow(height, 2));

	//Creamos un punto central y vamos lanzando radios en grados aleatorios y diferentes longitudes
	for (i = 0; i < num_vertices; i++) {

		//calculamos radios aleatorios aplicando un límite inferior y superior
		do {
			radius = random(max_radius);
		} while (radius < max_radius / 4);
		
		rad = (angle*i) / 180 * M_PI;

		dx = cos(rad) * radius;
		dy = sin(rad) * radius;

		figure.vertices[i].x = figure.center.x + dx;
		figure.vertices[i].y = figure.center.y + dy;
	}

	//Recalculamos el centro de gravedad del polígono según el resultado
	getFigureCenter(figure, &figure.center.x, &figure.center.y);

	return figure;
}


/* Crea la nave que maneja el jugador*/
tShip createShip(int width, int height) {
	tFigura figura;
	tShip ship;
	int posx, posy;
	int rgb[4] = { 255, 255, 255, 0 };
	
	figura.num_vertices = 4;
	figura.peso = 0;

	figura.center.x = posx = random(win_width);
	figura.center.y = posy = random(win_height);

	/*Especificamos la posición de los vértices*/
	figura.vertices[0].x = posx; figura.vertices[0].y = posy;
	figura.vertices[1].x = posx + width/2; figura.vertices[1].y = posy+height;
	figura.vertices[2].x = posx; figura.vertices[2].y = posy + height/2;
	figura.vertices[3].x = posx - width / 2; figura.vertices[3].y = posy + height;

	//Recalculamos el centro de gravedad de la nave según el resultado
	getFigureCenter(figura, &figura.center.x, &figura.center.y);

	ship.figura = figura;
	ship.accx = 0;
	ship.accy = 0;
	ship.dir = 0;

	return ship;
}


/* Crea todas las figuras que danzan por la pantalla */
void createFigures(tFigura figuras[50], int num_figuras) {
	int i;
	int width, height;
	int vertices;
	tFigura figura;

	for (i = 0; i < num_figuras; i++) {
		//comprobamos que cada figura sea de al menos 10px de lado
		do {
			width = random(80);
			height = random(80);
			vertices = random(11);
		} while (width < 10 || height < 10 || vertices<3);

		figuras[i] = createRandomFigure(width, height, vertices);
	}
}


/* Recalcula las coordenadas de los vértices del polígono, pasándole las nuevas coordenadas del nuevo centro 
Además, aplica la función de escalado indicada por el parámetro scale */
void recalculateCoordinates(tFigura(*figura), float newx, float newy, float scale) {
	int i;
	// Ésto guardará el conjunto de vectores de la figura, asumiendo que la figura más compleja será un decaedro
	tPoint vectores[20];

	//Hallamos los vectores del centro a los vértices y les aplicamos el escalado correspondiente
	for (i = 0; i < (*figura).num_vertices; i++) {
		vectores[i].x = ((*figura).center.x - (*figura).vertices[i].x) * scale;
		vectores[i].y = ((*figura).center.y - (*figura).vertices[i].y) * scale;
	}

	//El centro de la figura estará en las coordenadas indicadas como parámetros
	(*figura).center.x = newx;
	(*figura).center.y = newy;
	//Sumamos los vectores a los puntos actuales de la figura, tomando como nueva posición inicial los parámetros
	for (i = 0; i < (*figura).num_vertices; i++) {
		(*figura).vertices[i].x = (*figura).center.x - vectores[i].x;
		(*figura).vertices[i].y = (*figura).center.y - vectores[i].y;
	}
}


/* Ejecuta un rayCast sobre un polígono y devuelve si los puntos indicados se encuentran dentro del area del mismo */
int rayCast(tFigura figura, int x, int y) {
	int i, j, c = 0;

	for (i = 0, j = figura.num_vertices - 1; i < figura.num_vertices; j = i++) {
		if (((figura.vertices[i].y > y) != (figura.vertices[j].y > y)) &&
			(x < (figura.vertices[j].x - figura.vertices[i].x) * (y - figura.vertices[i].y) / (figura.vertices[j].y - figura.vertices[i].y) + figura.vertices[i].x))
			c = !c;
	}
	return c;
}


/* Aumenta el tamaño de la figura indicada */
void growFigure(tFigura(*figura), float scale) {
	recalculateCoordinates(figura, (*figura).center.x, (*figura).center.y, scale);
	(*figura).peso *= scale * 2;
}


/* Rota una figura los grados indicados */
void rotateFigure(tFigura(*figura), float degrees) {
	int i;
	int x, y;
	tPoint center[2];
	float dx, dy, r, a;

	for (i = 0; i < (*figura).num_vertices; i++) {
		dx = (*figura).vertices[i].x - (*figura).center.x;
		dy = (*figura).vertices[i].y - (*figura).center.y;
		r = sqrt(dx*dx + dy*dy);
		a = atan2(dy, dx);

		a -= degrees / 180 * M_PI;
		(*figura).vertices[i].x = (*figura).center.x + r*cos(a);
		(*figura).vertices[i].y = (*figura).center.y + r*sin(a);
	}
}


/* Comprueba si la pulsación del botón izquierdo del ratón ha ocurrido sobre una figura y la escala */
void getClickedFigure(tFigura figuras[50], int num_figuras) {
	int x, y;
	int i;
	tFigura figura;

	x = ESAT::MousePositionX();
	y = ESAT::MousePositionY();

	for (i = 0; i < num_figuras; i++) {
		figura = figuras[i];
		if (rayCast(figura, x, y)) {
			growFigure(&figura, 1.3);
			figuras[i] = figura;
		}
	}
}


/* Mueve todas las figuras verticalmente según su gravedad y horizontalmente según las teclas pulsadas */
void moveFigures(tFigura figuras[50], int num_figuras) {
	int i, j;
	//posiciones de la esquina superior izquierda
	float posx, posy;
	tFigura figura;
	//margen invisible en el exterior de la pantalla
	int margin = 25;

	//int rotate = ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Space);
	int rotate = ESAT::IsKeyDown(103);

	//Iteramos para todas las figuras existentes
	for (i = 0; i < num_figuras; i++) {
		figura = figuras[i];

		posx = figura.center.x;
		posy = figura.center.y;

		//Movemos cada uno de los vértices de la figura y su centro
		for (j = 0; j < figura.num_vertices; j++) {
			figura.vertices[j].y += figura.peso;

			//Detectamos la pulsación de las teclas de dirección laterales y movemos las figuras según corresponda
			if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Left)) {
				figura.vertices[j].x -= 5;
			}
			else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Right)) {
				figura.vertices[j].x += 5;
			}
		}

		//Movemos el centro de la figura
		figura.center.y += figura.peso;

		//Detectamos la pulsación de las teclas de dirección laterales y movemos las figuras según corresponda
		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Left)) {
			figura.center.x -= 5;
		}
		else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Right)) {
			figura.center.x += 5;
		}


		if (rotate) {
			rotateFigure(&figura, 3);
		}

		//Si se sale por la parte inferior de la pantalla, recalculamos sus coordenadas para colocarla en la parte superior de la misma
		if (posy > win_height) {
			recalculateCoordinates(&figura, posx, -margin, 1);
		}
		if (posx > win_width + margin) {
			recalculateCoordinates(&figura, -margin, posy, 1);
		}
		else if (posx < -margin) {
			recalculateCoordinates(&figura, win_width + margin - 1, posy, 1);
		}

		figuras[i] = figura;
	}
}


/* Escucha la pulsación de las teclas AWSD y mueve la nave según su aceleración almacenada */
void moveShip(tShip (*ship)) {
	int i;
	int margin = 25;
	float degrees = 5.0;
	float rad;
	float posx, posy;

	//A -> Rotar a la izquierda
	if (ESAT::IsKeyDown(97)) {
		rotateFigure(&(*ship).figura, degrees);
		(*ship).dir -= degrees;
	}
	//D -> Rotar a la derecha
	else if (ESAT::IsKeyDown(100)) {
		rotateFigure(&(*ship).figura, -degrees);
		(*ship).dir += degrees;
	}

	//W -> Avanzar
	if (ESAT::IsKeyDown(119)) {
		rad = (*ship).dir / 180 * M_PI;
		(*ship).accx += sin(rad);
		(*ship).accy -= cos(rad);
	}
	//S -> Retroceder
	else if (ESAT::IsKeyDown(115)) {
		rad = (*ship).dir / 180 * M_PI;
		(*ship).accx -= sin(rad);
		(*ship).accy += cos(rad);
	}

	//Avanzamos todos los vértices de la nave según la aceleración de la misma
	for (i = 0; i < (*ship).figura.num_vertices; i++) {
		(*ship).figura.vertices[i].x += 0.1*(*ship).accx;
		(*ship).figura.vertices[i].y += 0.1*(*ship).accy;
	}

	//Avanzamos su centro
	(*ship).figura.center.x += 0.1*(*ship).accx;
	(*ship).figura.center.y += 0.1*(*ship).accy;

	//Comprobamos si la figura se sale de la pantalla
	posx = (*ship).figura.center.x;
	posy = (*ship).figura.center.y;

	//Verticalmente
	if (posy > win_height + margin) {
		recalculateCoordinates(&(*ship).figura, posx, -margin, 1);
	}
	else if (posy < -margin) {
		recalculateCoordinates(&(*ship).figura, posx, win_height + margin, 1);
	}
	//Horizontalmente
	if (posx > win_width + margin) {
		recalculateCoordinates(&(*ship).figura, -margin, posy, 1);
	}
	else if (posx < -margin) {
		recalculateCoordinates(&(*ship).figura, win_width + margin - 1, posy, 1);
	}
}

/* Crea un disparo */
void shoot(tShip ship, tShot shots[50], int *num_shots) {
	tShot shot;

	shot.pos = ship.figura.vertices[0];
	shot.accx = ship.accx;
	shot.accy = ship.accy;

	shots[*num_shots] = shot;
	*num_shots++;
}


void moveShots(tShot shots[50], int num_shots) {
	int i;

	for (i = 0; i < num_shots; i++) {
		shots[i].pos.x += shots[i].accx;
		shots[i].pos.y += shots[i].accy;
	}
}


int ESAT::main(int argc, char **argv) {

	tFigura figuras[50];
	tShot shots[50];
	int num_shots = 0;
	tShip ship;
	int num_figuras = 20;

	srand(time(NULL));

	createFigures(figuras, num_figuras);
	ship = createShip(30, 60);

	ESAT::WindowInit(win_width, win_height);

	while (ESAT::WindowIsOpened() && !ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)){

		printFigure(ship.figura);

		printFigures(figuras, num_figuras, 1);

		printShots(shots, num_shots);

		moveFigures(figuras, num_figuras);

		moveShip(&ship);

		moveShots(shots, num_shots);

		if (ESAT::MouseButtonDown(1)) {
			getClickedFigure(figuras, num_figuras);
		}

		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Space)) {
			shoot(ship, shots, &num_shots);
		}

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}


	ESAT::WindowDestroy();
	return 0;
}