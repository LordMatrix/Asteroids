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
#include <stdio.h>

#include "lib/vectores.h"
#include "lib/funciones.h"
#include "lib/listaDoble.h"

#define PARTICLES_PER_BLAST 8

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
	int id;
	tShip ship;
	tShot iShots;
	int frame;
	int threshold;
	int points;
	int lives;

	char name[50];
	char birthDate[50];
	char province[50];
	char country[50];
	char user[50];
	char pass[50];
	char email[50];
	int credits;

	int bestScore;
	time_t date;
} tPlayer;

typedef struct {
	tFigura figura;
	float accx, accy;
	int type;
	int points;
	int state;
} tOvni;

typedef struct {
	char user[50];
	time_t date;
	int score;
} tHighScore;

typedef struct {
	tPlayer player;
	tOvni ovni;
	tAsteroid asteroids[100];
	int num_asteroids;
	int level;
	tShot shots[50];
	int num_shots;
} tSavedGame;

typedef struct {
	Point2 center;
	int age;
} tBlast;

int win_width = 1000, win_height = 600;
int margin = 25;

int level = 1;
int pause = false;
int player_improvedScore = 0;

tPlayer player;
tHighScore highScores[100];
int num_highScores = 0;

tAsteroid asteroids[100];
int num_asteroids;

tOvni ovni;

tShot shots[5000];
int num_shots = 0;

//Comprobación de tiempo transcurrido entre frames
clock_t t1 = clock(), t2 = clock();

tBlast blasts[20];
int num_blasts = 0;

typedef struct {
	Point2 pos;
	int size;
	std::string txt;
} tButton;

typedef struct {
	Point2 pos;
	int size;
	char txt[50];
	int txt_lenght;
	std::string title;
} tTextBox;

tButton buttons[10];
int num_buttons = 0;

tTextBox textBoxes[10];
int num_textBoxes = 0;


/* Crear botones del menú principal */
void initMainMenu() {
	num_buttons = 0;

	tButton b, c;
	initPoint2(&b.pos, 350, 230);
	b.txt = "LOG IN";
	b.size = 100;
	buttons[num_buttons] = b;
	num_buttons++;

	initPoint2(&c.pos, 350, 380);
	c.txt = "REGISTER";
	c.size = 100;
	buttons[num_buttons] = c;
	num_buttons++;
}

/* Crear botones del menú login/register */
void initRegisterMenu() {
	num_buttons = 0;

	tButton b, c;
	initPoint2(&b.pos, 150, 500);
	b.txt = "SAVE";
	b.size = 70;
	buttons[num_buttons] = b;
	num_buttons++;

	initPoint2(&c.pos, 600, 500);
	c.txt = "CANCEL";
	c.size = 70;
	buttons[num_buttons] = c;
	num_buttons++;
}


/* Crear botones del menú login/register */
void initLogInMenu() {
	num_buttons = 0;

	tButton b, c;
	initPoint2(&b.pos, 150, 500);
	b.txt = "LOG IN";
	b.size = 70;
	buttons[num_buttons] = b;
	num_buttons++;

	initPoint2(&c.pos, 600, 500);
	c.txt = "CANCEL";
	c.size = 70;
	buttons[num_buttons] = c;
	num_buttons++;
}


/* Crear botones del menú principal de usuario identificado */
void initLoggedInMenu() {
	num_buttons = 0;

	tButton b, c;
	initPoint2(&b.pos, 200, 250);
	b.txt = "NEW GAME";
	b.size = 90;
	buttons[num_buttons] = b;
	num_buttons++;

	initPoint2(&b.pos, 500, 250);
	b.txt = "LOAD GAME";
	b.size = 90;
	buttons[num_buttons] = b;
	num_buttons++;

	initPoint2(&c.pos, 200, 400);
	c.txt = "HIGHSCORES";
	c.size = 90;
	buttons[num_buttons] = c;
	num_buttons++;

	initPoint2(&c.pos, 500, 400);
	c.txt = "LOG OUT";
	c.size = 90;
	buttons[num_buttons] = c;
	num_buttons++;
}

void initHighScoresMenu() {
	num_buttons = 0;

	tButton b;
	initPoint2(&b.pos, 370, 500);
	b.txt = "RETURN";
	b.size = 80;
	buttons[num_buttons] = b;
	num_buttons++;
}

void initPauseMenu() {
	num_buttons = 0;

	tButton b, c;
	initPoint2(&b.pos, 350, 230);
	b.txt = "CONTINUE";
	b.size = 90;
	buttons[num_buttons] = b;
	num_buttons++;

	initPoint2(&c.pos, 350, 350);
	c.txt = "SAVE";
	c.size = 90;
	buttons[num_buttons] = c;
	num_buttons++;

	initPoint2(&c.pos, 350, 470);
	c.txt = "QUIT";
	c.size = 90;
	buttons[num_buttons] = c;
	num_buttons++;
}

/* Dibuja un cuadrado con su esquina superior izquierda en las coordenadas indicadas */
void drawButton(tButton button) {
	int padding = 30;
	int color[] = { 255, 255, 255, 255 };

	float pathPoints[] = { button.pos.x, button.pos.y,
		button.pos.x + button.size*2.5f, button.pos.y,
		button.pos.x + button.size*2.5f, button.pos.y + button.size,
		button.pos.x, button.pos.y + button.size,
		button.pos.x, button.pos.y
	};

	/*Color rgb interior del polígono*/
	ESAT::DrawSetFillColor(color[0], color[1], color[2], color[3]);

	/*Pinta la misma figura rellena. El último parámetro determina si se muestra el borde*/
	ESAT::DrawSolidPath(pathPoints, 5, true);

	/*Texto dentro del botón*/
	ESAT::DrawSetFillColor(0, 0, 0, 255);
	ESAT::DrawSetStrokeColor(0, 0, 0);
	/*Ajusta el tamaño de la fuente según la longitud del texto y el ancho del botón */
	int font = (button.size / button.txt.length()) * 2.5;
	ESAT::DrawSetTextSize(font);

	const char *txt2 = button.txt.c_str();
	ESAT::DrawText(button.pos.x + padding, button.pos.y + padding * 2, txt2);
}

//Comprueba y devuelve el número de botón pulsado
int checkButtonsClick() {
	int i, click = 0;

	for (i = 0; i < num_buttons && !click; i++) {
		//Comprobamos que el click sea en el interior del botón
		if (ESAT::MousePositionX() >(int)buttons[i].pos.x
			&& ESAT::MousePositionX() < (int)buttons[i].pos.x + 2 * buttons[i].size
			&& ESAT::MousePositionY() > (int)buttons[i].pos.y
			&& ESAT::MousePositionY() < (int)buttons[i].pos.y + buttons[i].size) {
			click = 1;
		}
	}
	return (click) ? i - 1 : -1;
}

void initPlayers() {
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
		if (((int)((*ship).accx + (*ship).accy) % 2) == 0)
			printThruster(&(*ship));
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

void removeBlast(int i) {
	blasts[i] = blasts[num_blasts - 1];
	num_blasts -= 1;
}

/* Desplaza los disparos y los hace desaparecer si han recorrido la distancia máxima */
void moveShots(tShot shots[50], int *num_shots) {
	int i;
	int max_age = 80;

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

/* Desplaza las partículas y las hace desaparecer si han recorrido la distancia máxima */
void moveBlasts() {
	int i, j;
	int max_age = 100;
	Point2 p;

	for (j = 0; j < num_blasts; j++) {
		for (i = 0; i < PARTICLES_PER_BLAST; i++) {
			p.x = blasts[j].center.x + (blasts[j].age / 2) * cos(rads((360 / PARTICLES_PER_BLAST)*i));
			p.y = blasts[j].center.y + (blasts[j].age / 2) * sin(rads((360 / PARTICLES_PER_BLAST)*i));

			ESAT::DrawLine(p.x, p.y, p.x + 1, p.y + 1);
		}
		
		blasts[j].age++;

		if (blasts[j].age > max_age) {
			removeBlast(j);
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

/* Crea una nueva explosión*/
void createBlast(Point2 p) {
	blasts[num_blasts].center = p;
	blasts[num_blasts].age = 0;
	num_blasts++;
}

/* Divide o elimina un asteroide */
void breakAsteroid(tAsteroid asteroids[50], int i, int *num_asteroids, int scores) {
	tAsteroid a, b;

	createBlast(asteroids[i].figura.center);
	
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
		asteroids[i] = asteroids[*num_asteroids - 1];
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
				breakAsteroid(asteroids, j, &(*num_asteroids), shots[i].owner);
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

	createBlast((*ship).figura.center);

	initPoint2(&p, win_width / 2, win_height / 2);

	(*ship).state = 2;
	(*ship).figura.age = 0;

	for (i = 0, j = 0; i < 8; i++, j += 2) {
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

	createBlast((*ovni).figura.center);

	initPoint2(&p, win_width / 2, win_height / 2);

	(*ovni).state = 2;
	(*ovni).figura.age = 0;
}

/* Dibuja y avanza la animaciónd e destrucción de la nave. Devuelve 1 si la animación ha terminado*/
void printShipDestruction(tShip *ship) {
	int i;
	Vec2 scaled_speed;
	float speed = 100;

	ESAT::DrawSetStrokeColor(255, 255, 255, 255);
	ESAT::DrawSetFillColor(0, 0, 0, 0);

	for (i = 0; i < 8; i += 2) {
		drawLine((*ship).destroyed[i], (*ship).destroyed[i + 1]);

		scaleVec2((*ship).destroyed_dir[i], 1 / speed, &scaled_speed);
		AddPoint2Vec2((*ship).destroyed[i], scaled_speed, &(*ship).destroyed[i]);
		scaleVec2((*ship).destroyed_dir[i + 1], 1 / speed, &scaled_speed);
		AddPoint2Vec2((*ship).destroyed[i + 1], scaled_speed, &(*ship).destroyed[i + 1]);
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

void checkShotsOvniHit(tShot shots[5000], int *num_shots, tOvni *ovni) {
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
	int i;
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
			shots[i].pos.x + shots[i].accx, shots[i].pos.y + shots[i].accy
			);
	}
}

/* Crea todos los asteroides */
void createAsteroids() {
	int i;
	num_asteroids = 0;
	Point2 origin;

	for (i = 0; i < ((level * 2) + 2) && num_asteroids <= 12; i++) {
		initPoint2(&origin, 0, 0);
		asteroids[num_asteroids] = createAsteroid(random(4) + 1, 1, origin);
		num_asteroids += 1;
	}
}

/* Mueve los asteroides */
void moveAsteroids() {
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
	int j;
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
void printAsteroids() {
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
void showFPS(float diff) {
	char buf[100];
	float fps;

	fps = CLOCKS_PER_SEC / diff;
	sprintf_s(buf, 100, "%.2f FPS", fps);
	ESAT::DrawText(650.0f, 30.0f, buf);
}

//Comprueba el tiempo transcurrido desde el último frame e indica si debe mostrarse el siguiente
int checkNextFrame(int show_fps) {
	float diff;
	int threshold = 14;

	t2 = clock();
	diff = (((float)t2 - (float)t1) / CLOCKS_PER_SEC) * 1000;
	
	if (diff > threshold) {
		if (show_fps)
			showFPS(diff);

		t1 = clock();
		return 0;
	}

	return 1;
}


void lightBox(char txt[]) {
	int i = 0;
	int ttl = 200;
	int size = 400;
	int y = 50;
	int x = 200;
	int fontAlpha, alpha = 0;
	int padding = 50;


	float pathPoints[] = { x, y,
		x + size*1.5f, y,
		x + size*1.5f, y + size,
		x, y + size,
		x, y
	};

	/*Color rgb interior del polígono*/

	int color[] = { 200, 200, 200, alpha };
	ESAT::DrawSetFillColor(color[0], color[1], color[2], color[3]);

	/*Pinta la misma figura rellena. El último parámetro determina si se muestra el borde*/
	ESAT::DrawSolidPath(pathPoints, 5, true);

	/*Texto dentro del botón*/
	ESAT::DrawSetFillColor(0, 0, 0, 255);
	ESAT::DrawSetStrokeColor(0, 0, 0);
	/*Ajusta el tamaño de la fuente según la longitud del texto y el ancho del botón */
	int font = (size / strlen(txt)) * 1.5;
	ESAT::DrawSetTextSize(font);

	//const char *txt2 = txt.c_str();
	ESAT::DrawText(x + padding, y + padding * 2, txt);


	while (ESAT::WindowIsOpened() && i < ttl) {

		if (checkNextFrame(0))
			continue;

		//La transparencia del lightbox cambia en cada frame
		if (i < ttl / 2 && alpha < 255)
			alpha++;
		else if (alpha > 0)
			alpha--;

		color[3] = alpha;
		ESAT::DrawSetFillColor(color[0], color[1], color[2], color[3]);
		ESAT::DrawSolidPath(pathPoints, 5, true);

		//Idem para el texto con colores inversos al fondo del lightbox
		fontAlpha = abs(0 + alpha);
		ESAT::DrawSetFillColor(fontAlpha, fontAlpha, fontAlpha, 255);
		ESAT::DrawSetStrokeColor(fontAlpha, fontAlpha, fontAlpha, 255);
		ESAT::DrawText(x + padding, y + padding * 2, txt);
		i++;

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}
}

//Guarda información sobre el jugador actual a disco
void updatePlayer() {
	FILE *f, *tmp;
	tPlayer temp;

	fopen_s(&tmp, "players.tmp", "wb");
	fopen_s(&f, "players.dat", "rb");

	do {
		fread(&temp, sizeof(tPlayer), 1, f);
		if (!feof(f))
			if (!(temp.id == player.id))
				fwrite(&temp, sizeof(tPlayer), 1, tmp);
			else
				fwrite(&player, sizeof(tPlayer), 1, tmp);
	} while (!feof(f));

	fclose(f);
	fclose(tmp);

	remove("players.dat");
	rename("players.tmp", "players.dat");
}


void saveGame() {
	tSavedGame game, temp;
	FILE *f, *tmp;
	int rcode = 0;
	
	std::memcpy(game.asteroids, asteroids, sizeof(tAsteroid)*100);
	std::memcpy(game.shots, shots, sizeof(tShot) * 50);	
	game.num_asteroids = num_asteroids;
	game.num_shots = num_shots;

	game.player = player;
	game.ovni = ovni;
	game.level = level;

	// Guardar juego en disco
	fopen_s(&tmp, "saveGames.tmp", "wb");
	rcode = fopen_s(&f, "saveGames.dat", "rb");

	//Si existe el fichero de partidas guardadas, se busca al jugador en él
	if (rcode != 2) {
		do {
			fread(&temp, sizeof(tSavedGame), 1, f);
			if (!feof(f))
				if (!(temp.player.id == player.id))
					fwrite(&temp, sizeof(tSavedGame), 1, tmp);
				else {
					fwrite(&game, sizeof(tSavedGame), 1, tmp);
					//Marcamos que la partida existe y se ha actualizado
					rcode = 1;
				}
		} while (!feof(f));
	}

	//Si el jugador no tiene una partida guardada, se añade
	if (!rcode) {
		fwrite(&game, sizeof(tSavedGame), 1, tmp);
	}

	//Cerramos este fichero en caso de que exista
	if (rcode != 2)
		fclose(f);

	fclose(tmp);

	remove("saveGames.dat");
	rename("saveGames.tmp", "saveGames.dat");

	lightBox("Game saved");
}

//Carga de disco una partida perteneciente al jugador actual.
//Devuelve 1 en caso de éxito, y 0 si no se ha encontrado partida guardada.
int loadGame() {
	tSavedGame game, temp;
	FILE *f, *tmp;
	int rcode = 0;

	rcode = fopen_s(&f, "saveGames.dat", "rb");

	//Si existe el fichero de partidas guardadas, se busca al jugador en él
	if (rcode != 2) {
		do {
			fread(&temp, sizeof(tSavedGame), 1, f);
			if (!feof(f))
				if (temp.player.id == player.id) {
					game = temp;
					rcode = 1;
				}
		} while (!feof(f) && !rcode);

		fclose(f);

		if (rcode == 1) {
			std::memcpy(asteroids, game.asteroids, sizeof(tAsteroid) * 100);
			std::memcpy(shots, game.shots, sizeof(tShot) * 50);
			num_asteroids = game.num_asteroids;
			num_shots = game.num_shots;

			player = game.player;
			ovni = game.ovni;
			level = game.level;
		}
	}

	return (rcode == 1);
}


int game() {
	int max_age = 100;
	int quit = 0;
	int game_over_age = 0;

	pause = false;

	while (ESAT::WindowIsOpened() && quit == 0) {

		if (checkNextFrame(1))
			continue;

		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Enter) || ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)) {
			initPauseMenu();
			pause = !pause;
		}
			
		if (pause && player.lives>0) {
			ESAT::DrawSetTextSize(70);
			ESAT::DrawText(300.0f, 160.0f, "PAUSED");
			drawButton(buttons[0]);
			drawButton(buttons[1]);
			drawButton(buttons[2]);
				
			if (ESAT::MouseButtonDown(1)) {
				switch (checkButtonsClick()) {
				case 0:
					pause = !pause;
					break;
				case 1:
					saveGame();
					break;
				case 2:
					quit = 1;
					break;
				default:
					break;
				}
			}
		}

		//Pausamos el juego y mostramos el mensaje de game over si al jugador no le quedan vidas
		if (player.lives < 1) {
			pause = true;
			if (game_over_age < 400) {
				ESAT::DrawSetTextSize(50);
				ESAT::DrawText(win_width / 3, win_height / 2, "GAME OVER");
				game_over_age++;
			}
			else {
				if (player.points > player.bestScore) {
					player_improvedScore = 1;
					player.bestScore = player.points;
					//Guardamos la fecha en formato UNIX
					time_t rawtime;
					time(&rawtime);
					player.date = rawtime;
					//Actualizamos datos del jugador
					updatePlayer();
				}
				quit = 1;
			}
		}

		ESAT::DrawSetTextSize(30);
		printInfo();

		if (ovni.state == 0) {
			printFigure(ovni.figura);

			if (!pause) {
				moveOvni(&ovni);
				ovni.figura.age++;
				//El ovni dispara cada X ciclos
				if (ovni.figura.age % 200 == 0)
					shoot(&player.ship, shots, &num_shots, &ovni);
			}
		}
		else if (!pause && player.ship.figura.age > 0 && player.ship.figura.age % 500 == 0 && num_asteroids<6)
			ovni = createOvni(random(2));

		switch (player.ship.state) {
		case 0:
			printFigure(player.ship.figura);

			if (!pause) {
				moveShip(&player.ship);

				if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Space) && num_shots<5) {
					shoot(&player.ship, shots, &num_shots);
				}
				checkHits(asteroids, &num_asteroids, shots, &num_shots, &player.ship, &ovni, ovni.state == 0);
				player.ship.figura.age++;
			}
			break;
		case 1:
			if (!pause) {
				if (player.ship.figura.age % 3 == 0)
					printFigure(player.ship.figura);

				moveShip(&player.ship);

				if (player.ship.figura.age > max_age) {
					player.ship.figura.age = 0;
					player.ship.state = 0;
				}
			}
			break;
		case 2:
			printShipDestruction(&player.ship);
			if (player.ship.figura.age > max_age) {
				player.ship.figura.age = 0;
				player.ship.state = 1;
			}
			break;
		}

		player.ship.figura.age++;

		printShots(shots, num_shots);
		printAsteroids();

		if (!pause) {
			moveShots(shots, &num_shots);
			moveAsteroids();
			moveBlasts();
		}

		if (num_asteroids == 0) {
			level++;
			createAsteroids();
		}
		

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}

	return 0;
}

/***********************************************************/
/************************* END GAME ************************/
/***********************************************************/


//Escucha los clicks del ratón en el menú de login/registro
int checkTextBoxesClick() {
	int i, click = 0;

	for (i = 0; i < num_textBoxes && !click; i++) {
		//Comprobamos que el click sea en el interior del botón
		if (ESAT::MousePositionX() >(int)textBoxes[i].pos.x
			&& ESAT::MousePositionX() < (int)textBoxes[i].pos.x + 8 * textBoxes[i].size
			&& ESAT::MousePositionY() > (int)textBoxes[i].pos.y
			&& ESAT::MousePositionY() < (int)textBoxes[i].pos.y + textBoxes[i].size) {
			click = 1;
		}
	}
	return (click) ? i - 1 : -1;
}


/* Dibuja una caja vacía sobre la que se escribirá texto */
void printTextBox(tTextBox box, int active) {
	int padding = 30;

	float pathPoints[] = { box.pos.x, box.pos.y,
		box.pos.x + box.size*8.0f, box.pos.y,
		box.pos.x + box.size*8.0f, box.pos.y + box.size,
		box.pos.x, box.pos.y + box.size,
		box.pos.x, box.pos.y
	};


	/*Color rgb interior del polígono*/
	if (active)
		ESAT::DrawSetFillColor(30, 30, 0);
	else
		ESAT::DrawSetFillColor(0, 0, 0);

	ESAT::DrawSetStrokeColor(255, 255, 255);

	/*Pinta la misma figura rellena. El último parámetro determina si se muestra el borde*/
	ESAT::DrawSolidPath(pathPoints, 5, true);

	/* Escribe el texto corrspondiente a la caja de texto actual */
	ESAT::DrawSetFillColor(150, 150, 150);
	ESAT::DrawText(box.pos.x, box.pos.y + 30, box.txt);
}


void textEditor(Point2 pos, char str[50], int *length) {
	int quit = 0;
	char key;

	key = ESAT::GetNextPressedKey();
	if (key > 0) {
		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Enter)) {
			quit = 1;
		}
		else if (ESAT::IsKeyDown(8) && *length > 0) {
			str[*length - 1] = '\0';
			*length -= 1;
		}
		else if (*length<13 && key != 9) {
			str[*length] = key;
			*length += 1;
			str[*length] = '\0';
		}
	}

}


void printTextBoxes(int active_box) {
	int is_active;

	for (int i = 0; i < num_textBoxes; i++) {
		is_active = (active_box == i) ? 1 : 0;
		ESAT::DrawSetFillColor(255, 255, 255);

		char title[50];//as 1 char space for null is also required
		strcpy_s(title, 50, textBoxes[i].title.c_str());

		ESAT::DrawText(130.0f, textBoxes[i].pos.y + 30, title);
		printTextBox(textBoxes[i], is_active);
	}
}


/* Crea las cajas de texto para los menús de login/registro */
void createTextBoxes(int option) {
	Point2 p;
	int posy = 80;
	int num_fields = 0;
	char titles[7][50];

	//Iniciamos los títulos para los campos de texto y los copiamos a la variable auxiliar que estará definida fuera del ámbito del bloque condicional
	if (option == 1) {
		char fields[7][50] = { "name", "birthDate", "province", "country", "user", "pass", "email" };
		num_fields = num_textBoxes = 7;
		std::memcpy(titles, fields, 350);
	}
	else {
		char fields[2][50] = { "Username", "Password" };
		num_fields = num_textBoxes = 2;
		std::memcpy(titles, fields, 350);
	}

	for (int i = 0; i < num_fields; i++) {
		initPoint2(&p, 500, posy);
		textBoxes[i].pos = p;
		textBoxes[i].size = 40;
		textBoxes[i].title = titles[i];
		strcpy_s(textBoxes[i].txt, 50, "");
		textBoxes[i].txt_lenght = 0;
		posy += 55;
	}
}

/*********************** FICHEROS ***************************/

//Devuelve el identificador del último registro del fichero
int getLastPlayerId() {
	FILE *f;
	tPlayer reg;
	int id;
	int rcode;

	rcode = fopen_s(&f, "players.dat", "r");

	reg.id = 0;

	if (rcode == 0) {
		do {
			fread(&reg, sizeof(tPlayer), 1, f);
		} while (!feof(f));

		fclose(f);
	}

	return reg.id;
}


//Leer datos del contacto desde los textBox y guardarlos en el tPlayer struct
void updatePlayerFromTextBoxes() {
	int bufferSize = 50;

	strcpy_s(player.name, bufferSize, textBoxes[0].txt);
	strcpy_s(player.birthDate, bufferSize, textBoxes[1].txt);
	strcpy_s(player.province, bufferSize, textBoxes[2].txt);
	strcpy_s(player.country, bufferSize, textBoxes[3].txt);
	strcpy_s(player.user, bufferSize, textBoxes[4].txt);
	strcpy_s(player.pass, bufferSize, textBoxes[5].txt);
	strcpy_s(player.email, bufferSize, textBoxes[6].txt);
}


void addPlayer() {
	FILE *f;

	player.credits = 10;
	player.bestScore = 0;
	player.date = 0;

	player.id = getLastPlayerId() + 1;

	fopen_s(&f, "players.dat", "ab");

	fwrite(&player, sizeof(tPlayer), 1, f);

	fclose(f);
}


int checkLogin() {
	FILE *f;
	tPlayer reg;

	int rcode;

	rcode = fopen_s(&f, "players.dat", "rb");

	if (rcode == 0) {
		rcode = 1;
		do {
			fread(&reg, sizeof(tPlayer), 1, f);
			//if (!feof(f)){
			if (strncmp(textBoxes[0].txt, reg.user, 50) == 0 && strncmp(textBoxes[1].txt, reg.pass, 50) == 0) {
				player = reg;
				rcode = 0;
			}
			//}
		} while (!feof(f) && rcode == 1);
		fclose(f);
	}


	return !rcode;
}

int checkPlayerExists() {
	FILE *f;
	tPlayer contact;
	int rcode;

	rcode = fopen_s(&f, "players.dat", "rb");

	if (rcode == 0) {
		rcode = 1;
		do {
			fread(&contact, sizeof(tPlayer), 1, f);
			if (!feof(f)){
				if (strcmp(contact.user, player.user) == 0) {
					rcode = 0;
				}
			}
		} while (!feof(f) && rcode == 1);
		fclose(f);
	}

	return (rcode == 0) ? 1 : 0;
}

/*********************** FIN DE FICHEROS ***************************/

//Ordena puntuaciones de mayor a menor con el método burbuja
void scoreBubbleSort()
{
	int c, d;
	tHighScore t;

	for (c = 0; c < (num_highScores - 1); c++)
	{
		for (d = 0; d < num_highScores - c - 1; d++)
		{
			if (highScores[d].score < highScores[d + 1].score)
			{
				/* Swapping */
				t = highScores[d];
				highScores[d] = highScores[d + 1];
				highScores[d + 1] = t;
			}
		}
	}
}

int fetchHighScores() {
	FILE *f;
	tPlayer reg;
	int rcode;
	int i = 0;

	num_highScores = 0;
	rcode = fopen_s(&f, "players.dat", "rb");

	//Escaneamos el fichero de jugadores recogiendo cada una de sus puntuaciones
	if (rcode == 0) {
		rcode = 1;
		do {
			fread(&reg, sizeof(tPlayer), 1, f);
			if (!feof(f)){
				//Almacenamos los datos de cada jugador en el array highScores
				strcpy_s(highScores[i].user, 50, reg.user);
				highScores[i].date = reg.date;
				highScores[i].score = reg.bestScore;
				num_highScores++;
			}
			i++;
		} while (!feof(f) && rcode == 1);
		fclose(f);
	}

	//Ordenamos el array por puntuación decreciente
	scoreBubbleSort();

	return !rcode;
}

void highScoresMenu() {
	int i;
	int quit = 0;
	int currentPlayer;
	float x, y;
	char score_str[20];
	char date_str[80];
	char top_str[5];
	struct tm timeinfo;

	fetchHighScores();
	initHighScoresMenu();

	while (ESAT::WindowIsOpened() && !quit) {

		if (checkNextFrame(0))
			continue;

		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape))
			quit = 1;

		y = 100.0f;
		x = 100.0f;

		ESAT::DrawSetTextSize(40);

		ESAT::DrawText(300.0f, 50.0f, "HIGHSCORES");

		ESAT::DrawSetTextSize(20);

		for (i = 0; i < num_highScores && i< 10; i++) {
			x = 60.0f;
			currentPlayer = (strcmp(player.user, highScores[i].user) == 0);

			if (currentPlayer) {
				//Aumentamos los créditos del jugador actual, si éste acaba de superar su puntuación y aparece en el listado de los 10 mejores
				if (player_improvedScore) {
					player_improvedScore = 0;
					player.credits += 5;
					updatePlayer();
					lightBox("You are awarded 5 extra credits!");
				}
				ESAT::DrawSetStrokeColor(50, 250, 50);
				ESAT::DrawSetFillColor(50, 250, 50);
			}

			//Top
			_snprintf_s(top_str, 30, "#%d", i + 1);
			ESAT::DrawText(x, y, top_str);
			x += 100;
			//Username
			ESAT::DrawText(x, y, highScores[i].user);
			x += 200;
			//Date
			localtime_s(&timeinfo, &highScores[i].date);
			strftime(date_str, 80, "%d-%m-%Y", &timeinfo);
			ESAT::DrawText(x, y, date_str);
			x += 350;
			//Score
			_itoa_s(highScores[i].score, score_str, 10);
			ESAT::DrawText(x, y, score_str);
			y += 30;

			//Reiniciamos el color de la fuente al final de cada iteración
			ESAT::DrawSetStrokeColor(255, 255, 255);
			ESAT::DrawSetFillColor(255, 255, 255);
		}

		drawButton(buttons[0]);

		if (ESAT::MouseButtonDown(1)) {
			switch (checkButtonsClick()) {
			case 0:
				quit = 1;
				break;
			default:
				break;
			}
		}

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}
}


void LoggedInMenu() {
	int quit = 0;
	char credits_str[30] = "Credits: ";
	char user_str[30] = "Logged in as: ";

	ESAT::DrawSetFillColor(255, 255, 255);
	ESAT::DrawSetStrokeColor(255, 255, 255);

	/* Iniciar cadenas para nombre de jugador y créditos disponibles */
	_snprintf_s(credits_str, 30, "Credits: %d", player.credits);
	_snprintf_s(user_str, 30, "Logged in as: %s", player.user);

	while (ESAT::WindowIsOpened() && quit == 0) {

		if (checkNextFrame(0))
			continue;

		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape))
			quit = 1;

		/* Mover asteroides por el fondo de la pantalla */
		moveAsteroids();
		printAsteroids();

		/* Indica el jugador identificado y sus créditos disponibles */
		
		ESAT::DrawSetFillColor(255, 255, 255);
		ESAT::DrawSetStrokeColor(255, 255, 255);
		ESAT::DrawSetTextSize(20);
		ESAT::DrawText(100.0f, 20.0f, user_str);
		ESAT::DrawText(650.0f, 20.0f, credits_str);

		ESAT::DrawSetTextSize(100);
		ESAT::DrawText(130.0f, 160.0f, "ASTEROIDS");

		drawButton(buttons[0]);
		drawButton(buttons[1]);
		drawButton(buttons[2]);
		drawButton(buttons[3]);

		if (ESAT::MouseButtonDown(1)) {
			switch (checkButtonsClick()) {
			case 0:
				if (player.credits > 0) {
					player.credits--;
					//Actualizar la cadena que muestra los créditos disponibles
					_snprintf_s(credits_str, 30, "Credits: %d", player.credits);
					//Guardar los cambios al jugador e iniciar el juego
					updatePlayer();
					//Iniciar variables de nueva partida
					initPlayers();
					player.ship = createShip(40, 50);
					createAsteroids();
					ovni = createOvni(1);
					ovni.state = 2;
					/********/
					game();
					//Reiniciamos el nivel y los asteroides al salir del juego de fondo de los menús
					level = 1;
					createAsteroids();
					//Mostrar las máximas puntuaciones al finalizar la partida
					highScoresMenu();
					//Actualiza el texto de créditos disponibles
					_snprintf_s(credits_str, 30, "Credits: %d", player.credits);
					//Reinicia los botones del menú de jugador identificado
					initLoggedInMenu();
				}
				else
					lightBox("Not Enough Credits");
				break;
			case 1:
				if (loadGame()) {
					game();
					createAsteroids();
					initHighScoresMenu();
					highScoresMenu();
					initLoggedInMenu();
				}
				else
					lightBox("There is no saved game");
				break;
			case 2:
				highScoresMenu();
				initLoggedInMenu();
				break;
			case 3:
				quit = 1;
				break;
			default:
				break;
			}
		}

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}

	initLogInMenu();
}


void logInMenu(int option) {
	int quit = 0;
	int active_box = -1;
	int key = 0;

	//Limpiar el buffer del teclado al entrar en este menú
	do {
		key = ESAT::GetNextPressedKey();
	} while (key > 0);

	createTextBoxes(option);

	if (option == 1)
		initRegisterMenu();
	else
		initLogInMenu();

	while (ESAT::WindowIsOpened() && quit == 0) {

		if (checkNextFrame(0))
			continue;

		printTextBoxes(active_box);

		moveAsteroids();
		printAsteroids();

		drawButton(buttons[0]);
		drawButton(buttons[1]);

		if (ESAT::MouseButtonDown(1)) {
			active_box = checkTextBoxesClick();
		}
		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Tab) && active_box<num_textBoxes) {
			fflush(stdin);
			active_box++;
		}
		if (active_box >= 0) {
			fflush(stdin);
			textEditor(textBoxes[active_box].pos, textBoxes[active_box].txt, &textBoxes[active_box].txt_lenght);
		}

		ESAT::DrawSetTextSize(30);
		ESAT::DrawSetFillColor(255, 255, 255);
		ESAT::DrawSetStrokeColor(255, 255, 255);

		/* Comprobar si se hace click sobre los botones de este menú */
		if (ESAT::MouseButtonDown(1)) {
			switch (checkButtonsClick()) {
			case 0:
				if (option == 1) {
					updatePlayerFromTextBoxes();

					if (!checkPlayerExists()) {
						addPlayer();
						lightBox("Player created");
					}
					else
						lightBox("Player already exists");
				}
				else {
					if (checkLogin()) {
						initLoggedInMenu();
						LoggedInMenu();
						//Al salir del menú de jugador identificado, volvemos directamente al menú principal
						quit = 1;
					}
					else
						lightBox("User/Password does not exist");
				}
				break;
			case 1:
				quit = 1;
				break;
			default:
				break;
			}
		}

		if (!ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)) {
			if (option == 0) {
				ESAT::DrawText(130.0f, 50.0f, "LOG IN");
			}
			else {
				ESAT::DrawText(130.0f, 50.0f, "REGISTER");
			}
		}
		else
			quit = 1;

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}

	initMainMenu();
}


void mainMenu() {

	ESAT::DrawSetTextSize(100);
	ESAT::DrawSetFillColor(255, 255, 255);
	ESAT::DrawSetStrokeColor(255, 255, 255);

	ESAT::DrawText(130.0f, 160.0f, "ASTEROIDS");

	drawButton(buttons[0]);
	drawButton(buttons[1]);

	if (ESAT::MouseButtonDown(1)) {
		switch (checkButtonsClick()) {
		case 0:
			logInMenu(0);
			break;
		case 1:
			logInMenu(1);
			break;
		default:
			break;
		}
	}
}


int ESAT::main(int argc, char **argv) {
	int exit = 0, start = 0;
	
	srand(time(NULL));

	ESAT::WindowInit(win_width, win_height);

	initText();
	initMainMenu();
	createAsteroids();

	while (ESAT::WindowIsOpened() && !exit) {

		if (checkNextFrame(0))
			continue;

		if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape))
			exit = 1;

		mainMenu();
		moveAsteroids();
		printAsteroids();

		ESAT::DrawClear(0, 0, 0);
		ESAT::WindowFrame();
	}

	ESAT::WindowDestroy();
	return 0;
}