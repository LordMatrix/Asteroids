/*
El clásico juego Asteroids, con registro y control de usuarios y puntuaciones.

Marcos Vázquez Rey
ESAT 2015
*/

#include <ESAT/window.h>
#include <ESAT/draw.h>
#include <ESAT/sprite.h>
#include <ESAT/input.h>

#include <windows.h>

#include <iosfwd>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "lib/vectores.h"
#include "lib/funciones.h"
#include "lib/listaDoble.h"

typedef struct {
	int num_vertices;
	Point2 vertices[30];
	Point2 center;
	int rgb[4];
	int age;
} tFigura;

typedef struct {
	tFigura figura;
	float accx, accy;
	float dir;
	tFigura thruster;
	// 0=normal, 1=invulnerable, 2=destruyéndose
	int state;
	//Guarda los pares de puntos que definen los vectores para la animación de la nave destruyéndose
	Point2 destroyed[8];
	//Guarda la dirección hacia la que se desplaza cada linea de la animación de nave destruyéndose
	Vec2 destroyed_dir[8];
} tShip;

typedef struct {
	Point2 pos;
	float accx, accy;
	int owner;
	int age;
} tShot;

typedef struct {
	tFigura figura;
	float accx, accy;
	int size;
	int type;
	int points;
	float angle;
} tAsteroid;

typedef struct {
	int code;
	tShip ship;
	tShot iShots;
	int frame;
	int threshold;
	int points;
	int lives;
} tPlayer;

typedef struct {
	tFigura figura;
	float accx, accy;
	int type;
	int points;
	int state;
} tOvni;

int win_width = 1000, win_height = 600;
int margin = 25;

int level = 1;
tPlayer player;


void initPlayers() {
	player.code = 0;
	player.threshold = 50;
	player.points = 0;
	player.lives = 3;
	player.frame = 0;
}

/* Dibuja una figura con su esquina superior izquierda en las coordenadas indicadas */
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

/* Rota una figura los grados indicados */
void rotateFigure(tFigura(*figura), float degrees) {
	int i;
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

/* Recalcula las coordenadas de los vértices del polígono, pasándole las nuevas coordenadas del nuevo centro
Además, aplica la función de escalado indicada por el parámetro scale */
void recalculateCoordinates(tFigura(*figura), Point2 newpos, float scale) {
	int i;
	// Ésto guardará el conjunto de vectores de la figura, asumiendo que la figura más compleja será un decaedro
	Vec2 vectores[20];

	//Hallamos los vectores del centro a los vértices y les aplicamos el escalado correspondiente
	for (i = 0; i < (*figura).num_vertices; i++) {
		vectores[i].x = ((*figura).center.x - (*figura).vertices[i].x) * scale;
		vectores[i].y = ((*figura).center.y - (*figura).vertices[i].y) * scale;
	}

	//El centro de la figura estará en las coordenadas indicadas como parámetros
	(*figura).center.x = newpos.x;
	(*figura).center.y = newpos.y;
	//Sumamos los vectores a los puntos actuales de la figura, tomando como nueva posición inicial los parámetros
	for (i = 0; i < (*figura).num_vertices; i++) {
		(*figura).vertices[i].x = (*figura).center.x - vectores[i].x;
		(*figura).vertices[i].y = (*figura).center.y - vectores[i].y;
	}
}

/* Dibuja la animación del motor en la parte trasera de la nave */
void printThruster(tShip(*ship)) {
	Point2 a, b;
	Vec2 v1, v2, v3;
	Point2 p1;

	(*ship).thruster.num_vertices = 3;

	a = (*ship).figura.vertices[2];
	b = (*ship).figura.vertices[3];
	initVec2FromPoints(&v1, a, b);
	scaleVec2(v1, 0.25, &v2);
	AddPoint2Vec2((*ship).figura.vertices[2], v2, &(*ship).thruster.vertices[0]);

	scaleVec2(v1, -0.25, &v2);
	AddPoint2Vec2((*ship).figura.vertices[3], v2, &(*ship).thruster.vertices[2]);


	initVec2FromPoints(&v1, (*ship).thruster.vertices[0], (*ship).thruster.vertices[2]);
	scaleVec2(v1, 0.5, &v2);
	AddPoint2Vec2((*ship).thruster.vertices[0], v2, &p1);

	perpendicularVec2(v1, &v3);
	scaleVec2(v3, -1, &v3);
	AddPoint2Vec2(p1, v3, &(*ship).thruster.vertices[1]);

	printFigure((*ship).thruster);
}

/*Comprueba si un punto está fuera del borde la pantalla y transforma p2 en el punto correcto.
Devuelve 1 si el punto se encontraba fuera de los límites, o 0 en caso contrario*/ 
int checkScreenBorders(Point2 p1, Point2 *p2) {
	//Comprobamos si la figura se sale de la pantalla
	float posx = p1.x;
	float posy = p1.y;
	int out = 0;

	//Verticalmente
	if (posy > win_height + margin) {
		initPoint2(&(*p2), posx, -margin);
		out = 1;
	}
	else if (posy < -margin) {
		initPoint2(&(*p2), posx, win_height + margin);
		out = 1;
	}
	//Horizontalmente
	if (posx > win_width + margin) {
		initPoint2(&(*p2), -margin, posy);
		out = 1;
	}
	else if (posx < -margin) {
		initPoint2(&(*p2), win_width + margin - 1, posy);
		out = 1;
	}

	return out;
}

/* Escucha la pulsación de las teclas AWSD y mueve la nave según su aceleración almacenada */
void moveShip(tShip(*ship)) {
	Point2 newpos;
	int i;
	float degrees = 6.0;

	//A -> Rotar a la izquierda
	if (ESAT::IsKeyPressed(97)) {
		rotateFigure(&(*ship).figura, degrees);
		(*ship).dir -= degrees;
	}
	//D -> Rotar a la derecha
	else if (ESAT::IsKeyPressed(100)) {
		rotateFigure(&(*ship).figura, -degrees);
		(*ship).dir += degrees;
	}

	//W -> Avanzar
	if (ESAT::IsKeyPressed(119)) {
		(*ship).accx += sin(rads((*ship).dir));
		(*ship).accy -= cos(rads((*ship).dir));
		//Pintamos el motor si la suma de las aceleraciones resulta en número par
		if ( ((int)( (*ship).accx + (*ship).accy ) % 2) == 0)
			printThruster(&(*ship));
	}

	//S -> Retroceder
	else if (ESAT::IsKeyPressed(115)) {
		(*ship).accx -= sin(rads((*ship).dir));
		(*ship).accy += cos(rads((*ship).dir));
	}

	//G -> Hiperespacio
	if (ESAT::IsKeyDown(103)) {
		initPoint2(&newpos, random(win_width), random(win_height));
		recalculateCoordinates(&(*ship).figura, newpos, 1);
	}

	//Avanzamos todos los vértices de la nave según la aceleración de la misma
	for (i = 0; i < (*ship).figura.num_vertices; i++) {
		(*ship).figura.vertices[i].x += 0.1*(*ship).accx;
		(*ship).figura.vertices[i].y += 0.1*(*ship).accy;
	}

	//Avanzamos su centro
	(*ship).figura.center.x += 0.1*(*ship).accx;
	(*ship).figura.center.y += 0.1*(*ship).accy;

	//Comprobamos si la nave ha salido de la pantalla	
	int out = checkScreenBorders((*ship).figura.center, &newpos);
	if (out)
		recalculateCoordinates(&(*ship).figura, newpos, 1);
	

	//Aplicamos una deceleración gradual en ambos ejes de movimiento
	if (abs((*ship).accx) > 0)
		(*ship).accx -= ((*ship).accx / 100);
	if (abs((*ship).accy) > 0)
		(*ship).accy -= ((*ship).accy / 100);
	
}

/* Calcula el centroide y devuelve los puntos x e y pertenecientes al centro del polígono */
void getFigureCenter(tFigura figura, Point2 *p) {

	int i = 0;

	float signedArea = 0.0;
	float x0 = 0.0; // Current vertex X
	float y0 = 0.0; // Current vertex Y
	float x1 = 0.0; // Next vertex X
	float y1 = 0.0; // Next vertex Y
	float a = 0.0;  // Partial signed area

	(*p).x = 0; (*p).y = 0;

	// para cada vértice salvo el último
	for (i = 0; i < figura.num_vertices - 1; ++i)
	{
		x0 = figura.vertices[i].x;
		y0 = figura.vertices[i].y;
		x1 = figura.vertices[i + 1].x;
		y1 = figura.vertices[i + 1].y;
		a = x0*y1 - x1*y0;
		signedArea += a;
		(*p).x += (x0 + x1)*a;
		(*p).y += (y0 + y1)*a;
	}

	// Último vértice
	x0 = figura.vertices[i].x;
	y0 = figura.vertices[i].y;
	x1 = figura.vertices[0].x;
	y1 = figura.vertices[0].y;
	a = x0*y1 - x1*y0;
	signedArea += a;
	(*p).x += (x0 + x1)*a;
	(*p).y += (y0 + y1)*a;

	signedArea *= 0.5;
	(*p).x /= (6 * signedArea);
	(*p).y /= (6 * signedArea);
}

/* Crea la figura de la nave, utilizada tanto para el control del jugador como para indicar el número de vidas*/
tFigura createShipFigure(int width, int height, Point2 center) {
	tFigura figura;

	figura.num_vertices = 5;

	figura.center = center;

	/*Especificamos la posición de los vértices*/
	figura.vertices[0].x = figura.center.x; figura.vertices[0].y = figura.center.y;
	figura.vertices[1].x = figura.center.x + width / 2; figura.vertices[1].y = figura.center.y + height;

	Vec2 v;
	initVec2FromPoints(&v, figura.vertices[0], figura.vertices[1]);
	scaleVec2(v, -0.25f, &v);
	AddPoint2Vec2(figura.vertices[1], v, &figura.vertices[2]);
	figura.vertices[4].x = figura.center.x - width / 2; figura.vertices[4].y = figura.center.y + height;

	initVec2FromPoints(&v, figura.vertices[0], figura.vertices[4]);
	scaleVec2(v, -0.25f, &v);
	AddPoint2Vec2(figura.vertices[4], v, &figura.vertices[3]);

	return figura;
}


/* Crea la nave que maneja el jugador*/
tShip createShip(int width, int height) {
	tFigura figura;
	tShip ship;
	Point2 center;

	int rgb[4] = { 255, 255, 255, 0 };

	initPoint2(&center, win_width / 2, win_height / 2);
	figura = createShipFigure(width, height, center);

	//Recalculamos el centro de gravedad de la nave según el resultado
	getFigureCenter(figura, &figura.center);

	ship.figura = figura;
	ship.accx = 0;
	ship.accy = 0;
	ship.dir = 0;
	ship.state = 0;
	ship.figura.age = 0;

	return ship;
}

/* Ejecuta un rayCast sobre un polígono y devuelve si los puntos indicados se encuentran dentro del area del mismo */
int rayCast(tFigura figura, Point2 p) {
	int i, j, c = 0;
	
	for (i = 0, j = figura.num_vertices - 1; i < figura.num_vertices; j = i++) {
		if (((figura.vertices[i].y > p.y) != (figura.vertices[j].y > p.y)) &&
			(p.x < (figura.vertices[j].x - figura.vertices[i].x) * (p.y - figura.vertices[i].y) / (figura.vertices[j].y - figura.vertices[i].y) + figura.vertices[i].x))
			c = !c;
	}
	return c;
}

/* Crea un disparo desde la nave */
void shoot(tShip *ship, tShot shots[50], int *num_shots) {
	tShot shot;
	float speed = 8.0f;

	(*ship).figura.age = 0;
	shot.pos = (*ship).figura.vertices[0];
	
	shot.accx = sin(rads((*ship).dir)) * speed;
	shot.accy = cos(rads((*ship).dir)) * -speed;
	shot.age = 0;
	shot.owner = 1;

	shots[*num_shots] = shot;
	*num_shots += 1;
  	speed = speed;
	
}

/* Crea un disparo desde el ovni */
void shoot(tShip *ship, tShot shots[50], int *num_shots, tOvni *ovni) {
	tShot shot;
	float speed = 8.0f;

	(*ovni).figura.age = 0;
	shot.pos = (*ovni).figura.center;

	if ((*ovni).type == 1) {
		shot.accx = sin(rads(random(360))) * random_sign() * speed;
		shot.accy = cos(rads(random(360))) * random_sign() * speed;
	}
	else {
		Vec2 v;
		initVec2FromPoints(&v, (*ovni).figura.center, (*ship).figura.center);
		normalizeVec2(v, &v);
		
		shot.accx = (v.x) * speed;
		shot.accy = (v.y) * speed;
	}

	shot.age = 0;
	shot.owner = 0;

	shots[*num_shots] = shot;
	*num_shots += 1;
	speed = speed;
}

//Elimina un disparo del vector de disparos
void removeShot(tShot shots[50], int i, int *num_shots) {
	shots[i] = shots[*num_shots - 1];
	*num_shots -= 1;
}

/* Desplaza los disparos y los hace desaparecer si han recorrido la distancia máxima */
void moveShots(tShot shots[50], int *num_shots) {
	int i;
	int max_age = 700;

	for (i = 0; i < *num_shots; i++) {
		shots[i].pos.x += shots[i].accx;
		shots[i].pos.y += shots[i].accy;
		shots[i].age++;

		checkScreenBorders(shots[i].pos, &shots[i].pos);
		if (shots[i].age > max_age) {
			removeShot(shots, i, &(*num_shots));
		}
	}
}

/* Crea un asteroide */
tAsteroid createAsteroid(int type, int size, Point2 origin) {
	tAsteroid a;

	a.type = type;
	a.size = size;

	switch (type) {
	case 1:
		a.figura.num_vertices = 12;
		initPoint2(&a.figura.vertices[0], 11, 1);
		initPoint2(&a.figura.vertices[1], 16, 6);
		initPoint2(&a.figura.vertices[2], 24, 1);
		initPoint2(&a.figura.vertices[3], 31, 9);
		initPoint2(&a.figura.vertices[4], 23, 13);
		initPoint2(&a.figura.vertices[5], 31, 21);
		initPoint2(&a.figura.vertices[6], 22, 31);
		initPoint2(&a.figura.vertices[7], 13, 26);
		initPoint2(&a.figura.vertices[8], 9, 31);
		initPoint2(&a.figura.vertices[9], 2, 23);
		initPoint2(&a.figura.vertices[10], 6, 17);
		initPoint2(&a.figura.vertices[11], 2, 10);
		break;
	case 2:
		a.figura.num_vertices = 11;
		initPoint2(&a.figura.vertices[0], 12, 1);
		initPoint2(&a.figura.vertices[1], 24, 1);
		initPoint2(&a.figura.vertices[2], 31, 11);
		initPoint2(&a.figura.vertices[3], 31, 17);
		initPoint2(&a.figura.vertices[4], 20, 30);
		initPoint2(&a.figura.vertices[5], 13, 30);
		initPoint2(&a.figura.vertices[6], 14, 18);
		initPoint2(&a.figura.vertices[7], 7, 29);
		initPoint2(&a.figura.vertices[8], 2, 19);
		initPoint2(&a.figura.vertices[9], 8, 15);
		initPoint2(&a.figura.vertices[10], 2, 11);
		break;
	case 3:
		a.figura.num_vertices = 10;
		initPoint2(&a.figura.vertices[0], 8, 1);
		initPoint2(&a.figura.vertices[1], 15, 7);
		initPoint2(&a.figura.vertices[2], 23, 1);
		initPoint2(&a.figura.vertices[3], 30, 7);
		initPoint2(&a.figura.vertices[4], 25, 14);
		initPoint2(&a.figura.vertices[5], 29, 22);
		initPoint2(&a.figura.vertices[6], 19, 30);
		initPoint2(&a.figura.vertices[7], 6, 30);
		initPoint2(&a.figura.vertices[8], 1, 25);
		initPoint2(&a.figura.vertices[9], 1, 7);
		break;
	case 4:
	default:
		a.figura.num_vertices = 12;
		initPoint2(&a.figura.vertices[0], 7, 0);
		initPoint2(&a.figura.vertices[1], 18, 0);
		initPoint2(&a.figura.vertices[2], 30, 8);
		initPoint2(&a.figura.vertices[3], 30, 11);
		initPoint2(&a.figura.vertices[4], 18, 15);
		initPoint2(&a.figura.vertices[5], 29, 22);
		initPoint2(&a.figura.vertices[6], 22, 29);
		initPoint2(&a.figura.vertices[7], 17, 24);
		initPoint2(&a.figura.vertices[8], 7, 29);
		initPoint2(&a.figura.vertices[9], 0, 18);
		initPoint2(&a.figura.vertices[10], 1, 7);
		initPoint2(&a.figura.vertices[11], 12, 7);
		break;
	}

	getFigureCenter(a.figura, &a.figura.center);


	switch (size) {
	case 1:
		a.points = 20;
		break;
	case 2:
		a.points = 50;
		break;
	case 3:
	default:
		a.points = 100;
		break;
	}

	//Aceleraciones aleatorias
	int speed = 30;
	a.accx = random(speed) * random_sign();
	a.accy = random(speed - abs(a.accx)) * random_sign();

	//punto de origen del asteroide
	recalculateCoordinates(&a.figura, origin, 4 - size);

	//Creamos una rotación aleatoria y rotamos los vértices
	a.angle = random(360);
	rotateFigure(&a.figura, a.angle);

	return a;
}

/* Crea un ovni */
tOvni createOvni(int type) {
	tOvni o;
	Point2 p;
	float size;

	initPoint2(&p, 200, 200);

	//Aceleraciones aleatorias
	int speed = 30;
	o.accx = random(speed) * random_sign();
	o.accy = random(speed - abs(o.accx)) * random_sign();

	o.type = type;
	if (o.type == 1) {
		size = 2;
		o.points = 200;
	}
	else {
		size = 1;
		o.points = 1000;
	}

	o.state = 0;
	o.figura.num_vertices = 12;

	//Definición de los vértices de la figura ovni
	initPoint2(&o.figura.vertices[0], 2, 15);
	initPoint2(&o.figura.vertices[1], 14, 23);
	initPoint2(&o.figura.vertices[2], 36, 23);
	initPoint2(&o.figura.vertices[3], 48, 15);
	initPoint2(&o.figura.vertices[4], 36, 8);
	initPoint2(&o.figura.vertices[5], 31, 1);
	initPoint2(&o.figura.vertices[6], 19, 1);
	initPoint2(&o.figura.vertices[7], 14, 8);
	initPoint2(&o.figura.vertices[8], 36, 8);
	initPoint2(&o.figura.vertices[9], 14, 8);
	initPoint2(&o.figura.vertices[10], 2, 15);
	initPoint2(&o.figura.vertices[11], 48, 15);

	getFigureCenter(o.figura, &o.figura.center);
	recalculateCoordinates(&o.figura, p, size);

	return o;
}

/* Divide o elimina un asteroide */
void breakAsteroid(tAsteroid asteroids[50], int i, int *num_asteroids, int scores) {
	tAsteroid a, b;

	if (scores) {
		player.points += asteroids[i].points;
	}

	if (asteroids[i].size < 3) {

		a = createAsteroid(random(4) + 1, asteroids[i].size + 1, asteroids[i].figura.center);
		b = createAsteroid(random(4) + 1, asteroids[i].size + 1, asteroids[i].figura.center);

		asteroids[i] = a;
		asteroids[*num_asteroids] = b;
		*num_asteroids += 1;
	}
	else {
		asteroids[i] = asteroids[*num_asteroids-1];
		*num_asteroids -= 1;
	}
}

/* Itera sobre los asteroides y disparos, comprobando si alguno de ellos ha impactado */
void checkShotsAsteroidsHit(tAsteroid asteroids[50], int *num_asteroids, tShot shots[5000], int *num_shots) {
	int i, j;
	int hit = 0;

	for (i = 0; i < *num_shots; i++) {
		hit = 0;
		for (j = 0; j < *num_asteroids && !hit; j++) {
			if (rayCast(asteroids[j].figura, shots[i].pos) == 1) {
				//Partimos el asteroide
				breakAsteroid(asteroids, j, &(*num_asteroids), 1);
				//Eliminamos el disparo
				removeShot(shots, i, &(*num_shots));
				hit = 1;
			}
		}

	}
}


/* Marca la nave como destruída e inicia los vectores del efecto gráfico de destrucción*/
void destroyShip(tShip *ship) {
	Point2 p;
	int i, j;
	Vec2 dir;

	initPoint2(&p, win_width/2, win_height/2);
	
	(*ship).state = 2;
	(*ship).figura.age = 0;

	for (i = 0, j = 0; i < 8; i++, j+=2) {
		initVec2(&dir, random(20) * random_sign(), random(20) * random_sign());
		(*ship).destroyed_dir[i] = dir;
		initPoint2(&(*ship).destroyed[i], (*ship).figura.center.x + dir.x, (*ship).figura.center.y + dir.y);
	}

	recalculateCoordinates(&(*ship).figura, p, 1);
	(*ship).accx = 0;
	(*ship).accy = 0;
}

void destroyOvni(tOvni *ovni) {
	Point2 p;
	int i, j;
	Vec2 dir;

	initPoint2(&p, win_width / 2, win_height / 2);

	(*ovni).state = 2;
	(*ovni).figura.age = 0;
}

/* Dibuja y avanza la animaciónd e destrucción de la nave. Devuelve 1 si la animación ha terminado*/
void printShipDestruction(tShip *ship) {
	int i;
	Point2 p1, p2;
	Vec2 scaled_speed;
	float speed = 100;

	ESAT::DrawSetStrokeColor(255, 255, 255, 255);
	ESAT::DrawSetFillColor(0, 0, 0, 0);
	
	for (i = 0; i < 8; i += 2) {
		drawLine((*ship).destroyed[i], (*ship).destroyed[i + 1]);
		
		scaleVec2((*ship).destroyed_dir[i], 1 / speed, &scaled_speed);
		AddPoint2Vec2((*ship).destroyed[i], scaled_speed, &(*ship).destroyed[i]);
		scaleVec2((*ship).destroyed_dir[i+1], 1 / speed, &scaled_speed);
		AddPoint2Vec2((*ship).destroyed[i+1], scaled_speed, &(*ship).destroyed[i+1]);
	}
}

void checkShipAsteroidsHit(tAsteroid asteroids[50], int *num_asteroids, tShip *ship) {
	int i, j;
	int hit = 0;

	for (i = 0; i < (*ship).figura.num_vertices && !hit; i++) {
		hit = 0;
		for (j = 0; j < *num_asteroids && !hit; j++) {
			if (rayCast(asteroids[j].figura, (*ship).figura.vertices[i]) == 1) {
				//Partimos el asteroide
				breakAsteroid(asteroids, j, &(*num_asteroids), 0);
				//Destruimos la nave
				destroyShip(&(*ship));
				hit = 1;
				player.lives--;
			}
		}

	}
}

void checkAsteroidsOvniHit(tAsteroid asteroids[50], int *num_asteroids, tOvni *ovni){
	int i, j;
	int hit = 0;

	for (i = 0; i < (*ovni).figura.num_vertices && !hit; i++) {
		hit = 0;
		for (j = 0; j < *num_asteroids && !hit; j++) {
			if (rayCast(asteroids[j].figura, (*ovni).figura.vertices[i]) == 1) {
				//Partimos el asteroide
				breakAsteroid(asteroids, j, &(*num_asteroids), 0);
				//Destruimos el ovni
				destroyOvni(&(*ovni));
			}
		}

	}
}

void checkShotsOvniHit (tShot shots[5000], int *num_shots, tOvni *ovni) {
	int i;
	int hit = 0;

	for (i = 0; i < *num_shots && !hit; i++) {
		if (shots[i].owner == 1 && rayCast((*ovni).figura, shots[i].pos) == 1) {
			//Destruimos el ovni
			destroyOvni(&(*ovni));
			//Eliminamos el disparo
			removeShot(shots, i, &(*num_shots));
			player.points += (*ovni).points;
			hit = 1;
		}
	}
}

void checkShotsShipHit(tShot shots[5000], int *num_shots, tShip *ship) {
	int i;
	int hit = 0;

	for (i = 0; i < *num_shots && !hit; i++) {
		if (shots[i].owner == 0 && rayCast((*ship).figura, shots[i].pos) == 1) {
			//Eliminamos el disparo
			removeShot(shots, i, &(*num_shots));
			//Destruimos la nave
			destroyShip(&(*ship));
			hit = 1;
			player.lives--;
		}
	}
}

void checkShipOvniHit(tShip *ship, tOvni *ovni) {
	int i, j;
	int hit = 0;

	for (i = 0; i < (*ship).figura.num_vertices && !hit; i++) {
		hit = 0;
		if (rayCast((*ovni).figura, (*ship).figura.vertices[i]) == 1) {
			//Partimos el asteroide
			destroyOvni(&(*ovni));
			//Destruimos la nave
			destroyShip(&(*ship));
			hit = 1;
			player.lives--;
		}
	}
}

/* Itera sobre los elementos que pueden colisionar entre ellos y hace las comprobaciones pertinentes */
void checkHits(tAsteroid asteroids[50], int *num_asteroids, tShot shots[5000], int *num_shots, tShip *ship, tOvni *ovni, int check_ovni) {
	
	checkShotsAsteroidsHit(asteroids, &(*num_asteroids), shots, &(*num_shots));
	checkShipAsteroidsHit(asteroids, &(*num_asteroids), &(*ship));

	if (check_ovni) {
		checkAsteroidsOvniHit(asteroids, &(*num_asteroids), &(*ovni));
		checkShotsOvniHit(shots, &(*num_shots), &(*ovni));
		checkShipOvniHit(&(*ship), &(*ovni));
		checkShotsShipHit(shots, &(*num_shots), &(*ship));
	}
}

/* Dibuja los disparos */
void printShots(tShot shots[50], int num_shots) {
	int i;

	for (i = 0; i < num_shots; i++) {
		ESAT::DrawLine(
			shots[i].pos.x, shots[i].pos.y,
			shots[i].pos.x + 1, shots[i].pos.y + 1
		);
	}
}

/* Crea todos los asteroides */
void createAsteroids(tAsteroid asteroids[500], int *num_asteroids) {
	int i;
	Point2 origin;

	for (i = 0; i < ((level * 2) + 2) && *num_asteroids<=12; i++) {
		initPoint2(&origin, 0, 0);
		asteroids[*num_asteroids] = createAsteroid(random(4)+1, 1, origin);
		*num_asteroids += 1;
	}
}

/* Mueve los asteroides */
void moveAsteroids(tAsteroid asteroids[50], int num_asteroids) {
	int i, j;
	Point2 newpos;
	
	for (i = 0; i < num_asteroids; i++) {
		//Avanzamos todos los vértices del asteroide según la aceleración del mismo
		for (j = 0; j < asteroids[i].figura.num_vertices; j++) {
			asteroids[i].figura.vertices[j].x += 0.1 * asteroids[i].accx;
			asteroids[i].figura.vertices[j].y += 0.1 * asteroids[i].accy;
		}

		//Avanzamos su centro
		asteroids[i].figura.center.x += 0.1 * asteroids[i].accx;
		asteroids[i].figura.center.y += 0.1 * asteroids[i].accy;

		//Comprobamos si la nave ha salido de la pantalla	
		int out = checkScreenBorders(asteroids[i].figura.center, &newpos);
		if (out)
			recalculateCoordinates(&asteroids[i].figura, newpos, 1);
	}
}

void moveOvni(tOvni *ovni) {
	int i, j;
	Point2 newpos;

	//Avanzamos todos los vértices del asteroide según la aceleración del mismo
	for (j = 0; j < (*ovni).figura.num_vertices; j++) {
		(*ovni).figura.vertices[j].x += 0.1 * (*ovni).accx;
		(*ovni).figura.vertices[j].y += 0.1 * (*ovni).accy;
	}

	//Avanzamos su centro
	(*ovni).figura.center.x += 0.1 * (*ovni).accx;
	(*ovni).figura.center.y += 0.1 * (*ovni).accy;

	//Comprobamos si la nave ha salido de la pantalla	
	int out = checkScreenBorders((*ovni).figura.center, &newpos);
	if (out)
		recalculateCoordinates(&(*ovni).figura, newpos, 1);
}


/* Dibuja cada uno de los asteroides */
void printAsteroids(tAsteroid asteroids[50], int num_asteroids) {
	int i;

	for (i = 0; i < num_asteroids; i++) {
		printFigure(asteroids[i].figura);
	}
}

void initText() {
	ESAT::DrawSetTextFont("assets/ca.ttf");
	ESAT::DrawSetTextSize(20);
	ESAT::DrawSetFillColor(255, 255, 255);
	ESAT::DrawSetStrokeColor(255, 255, 255);
}

void printInfo() {
	char str[20];
	int i;
	tFigura figure;
	Point2 center;

	//Puntos
	_itoa_s(player.points, str, 10);
	ESAT::DrawText(100.0f, 30.0f, str);
	//Nivel
	ESAT::DrawText(350.0f, 30.0f, "Lvl. ");
	_itoa_s(level, str, 10);
	ESAT::DrawText(470.0f, 30.0f, str);
	//Vidas
	for (i = 0; i < player.lives; i++) {
		initPoint2(&center, 100.0f + (i * 40), 50.0f);
		figure = createShipFigure(20, 25, center);
		printFigure(figure);
	}
}

/* Comprueba si debe procesarse el siguiente frame y actualiza los FPS */
int showNextFrame(float diff, float *fps) {
	char buf[100];
	int t1 = (int)clock();
	
	if (t1 % 500 == 0) {
		*fps = 1000 / diff;
	}

	sprintf_s(buf, 100, "%.2f FPS", *fps);
	ESAT::DrawText(650.0f, 30.0f, buf);

	if (diff < 14)
		return 0;
	 
	

	return 1;
}

int game() {
	tShip ship;
	tOvni ovni;
	tShot shots[5000];
	tAsteroid asteroids[500];
	clock_t t1 = clock(), t2 = clock();
	int num_shots = 0;
	int num_asteroids = 0;
	int max_age = 100;
	float diff, fps;
	int quit = 0;

	srand(time(NULL));

	initPlayers();

	ship = createShip(40, 50);
	createAsteroids(asteroids, &num_asteroids);

	/* Ovni */
	ovni = createOvni(1);
	ovni.state = 2;
	/********/

	while (ESAT::WindowIsOpened() && quit == 0) {
		if (!ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)) {
			/*** Comprueba el tiempo transcurrido desde el último frame ***/
			t2 = clock();
			diff = (((float)t2 - (float)t1) / CLOCKS_PER_SEC) * 1000;
			if (!showNextFrame(diff, &fps))
				continue;
			t1 = clock();
			/**************************************************************/

			printInfo();

			if (ovni.state == 0) {
				printFigure(ovni.figura);
				moveOvni(&ovni);
				ovni.figura.age++;
				if (ovni.figura.age % 200 == 0)
					shoot(&ship, shots, &num_shots, &ovni);
			}
			else if (ship.figura.age % 500 == 0)
				ovni = createOvni(random(2));

			switch (ship.state) {
			case 0:
				printFigure(ship.figura);
				moveShip(&ship);
				if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Space)) {
					shoot(&ship, shots, &num_shots);
				}
				checkHits(asteroids, &num_asteroids, shots, &num_shots, &ship, &ovni, ovni.state == 0);
				ship.figura.age++;
				break;
			case 1:
				if (ship.figura.age % 3 == 0)
					printFigure(ship.figura);

				moveShip(&ship);

				if (ship.figura.age > max_age) {
					ship.figura.age = 0;
					ship.state = 0;
				}
				break;
			case 2:
				printShipDestruction(&ship);
				if (ship.figura.age > max_age) {
					ship.figura.age = 0;
					ship.state = 1;
				}
				break;
			}

			ship.figura.age++;

			printShots(shots, num_shots);
			moveShots(shots, &num_shots);

			moveAsteroids(asteroids, num_asteroids);
			printAsteroids(asteroids, num_asteroids);


			if (num_asteroids == 0) {
				level++;
				createAsteroids(asteroids, &num_asteroids);
			}
		} else {
			quit = 1;
		}

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}

	return 0;
}

int ESAT::main(int argc, char **argv) {
	int exit, start = 0;

	ESAT::WindowInit(win_width, win_height);

	initText();

	//while (ESAT::WindowIsOpened() && !exit) {

	//if (start)
	game();
	//else
	//Mainmenu();

	ESAT::DrawClear(0, 0, 0);
	ESAT::WindowFrame();
	//}

	ESAT::WindowDestroy();
	return 0;
}