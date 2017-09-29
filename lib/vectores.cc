/* TODO:
Dibujar un cuadrado rotando en el centro de la pantalla.
Implementado con vectores

- Pinta un monigote y después lo pinta transformado (rotado y escalado... ) usando la función anterior para pintarlo, y una única matriz de escalado y rotado precalculada


*/

#include <MOMOS/window.h>
#include <MOMOS/draw.h>
#include <MOMOS/sprite.h>
#include <MOMOS/input.h>

#include <Windows.h>

#include <iosfwd>
#include <string>

#define DrawText DrawText
#define M_PI 3.14159265358979323846;

typedef struct {
	float x;
	float y;
} Point2, Vec2;

typedef struct {
	float x;
	float y;
	float z;
} Vec3;

typedef float Mat2[2][2];
typedef float Mat3[3][3];


void initVec2(Vec2 *v, float x, float y) {
	(*v).x = x;
	(*v).y = y;
}

void initVec3(Vec3 *v, float x, float y) {
	(*v).x = x;
	(*v).y = y;
	(*v).z = 1;
}

void initPoint2(Point2 *p, float x, float y) {
	(*p).x = x;
	(*p).y = y;
}

void initMat2(Mat2 *m, float w, float x, float y, float z) {
	(*m)[0][0] = w;
	(*m)[0][1] = x;
	(*m)[1][0] = y;
	(*m)[1][1] = z;
}

void intMat2FromBasis(Mat2 m, Vec2 a1, Vec2 a2) {}

void initVec2FromPoints(Vec2 *v, Point2 a, Point2 b) {
	(*v).x = b.x - a.x;
	(*v).y = b.y - a.y;
}


float Dot(Vec2 v1, Vec2 v2) {
	return (v1.x * v2.x + v1.y * v2.y);
}

void AddPoint2Vec2(Point2 p1, Vec2 v1, Point2 *p2) {
	(*p2).x = p1.x + v1.x;
	(*p2).y = p1.y + v1.y;
}

void AddVec2(Vec2 v1, Vec2 v2, Vec2 *v3) {
	(*v3).x = v1.x + v2.x;
	(*v3).y = v1.y + v2.y;
}

void SubstractVec2(Vec2 v1, Vec2 v2, Vec2 *v3) {
	(*v3).x = v1.x - v2.x;
	(*v3).y = v1.y - v2.y;
}

void scaleVec2(Vec2 v1, float n, Vec2 *v2) {
	(*v2).x = v1.x * n;
	(*v2).y = v1.y * n;
}

void perpendicularVec2(Vec2 v1, Vec2 *v2) {
	(*v2).x = v1.y * -1;
	(*v2).y = v1.x;
}

float LenghtVec2(Vec2 v1) {
	return sqrt(pow(v1.x, 2) + pow(v1.y, 2));
}

void normalizeVec2(Vec2 v1, Vec2 *v2) {
	float length = LenghtVec2(v1);

	(*v2).x = v1.x / length;
	(*v2).y = v1.y / length;
}

void projectionVec2(Vec2 v1, Vec2 v2, Vec2 *v3) {
	float factor = Dot(v1, v2) / pow(LenghtVec2(v2), 2);

	(*v3).x = v2.x * factor;
	(*v3).y = v2.y * factor;
}

void combinePoint2(Point2 v1, Point2 v2, float f, Point2*) {

}

void multMat2(Mat2 m1, Mat2 m2, Mat2 *m3) {
	(*m3)[0][0] = m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0];
	(*m3)[0][1] = m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1];
	(*m3)[1][0] = m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0];
	(*m3)[1][1] = m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1];
}

void multMat2Vec2(Mat2 m1, Vec2 v1, Vec2 *v2) {
	(*v2).x = m1[0][0] * v1.x + m1[0][1] * v1.y;
	(*v2).y = m1[1][0] * v1.x + m1[1][1] * v1.y;
}


void addMat2(Mat2 m1, Mat2 m2, Mat2 *m3) {
	int i, j;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			(*m3)[i][j] = m1[i][j] + m2[i][j];
		}
	}
}

void scaleMat2(Mat2 m1, float n, Mat2 *m2) {
	int i, j;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			(*m2)[i][j] = m1[i][j] * n;
		}
	}
}

float rads(float degrees) {
	float rads;
	rads = (degrees) / 180 * M_PI;
	return rads;
}

void DrawVec2(Vec2 v, Point2 p) {
	Point2 a, b;
	Vec2 pp;
	float arrow_height = 20.0f;
	float pathPoints[10];

	a = p;
	initPoint2(&b, p.x + v.x, p.y + v.y);

	MOMOS::DrawSetStrokeColor(255, 255, 255, 255);
	MOMOS::DrawLine(a.x, a.y, b.x, b.y);

	perpendicularVec2(v, &pp);

}


/*********************** VARIABLES ***************************/
//int win_width = 1000, win_height = 600;


/* Dibuja un cuadrado con su esquina superior izquierda en las coordenadas indicadas */
void printSquare(Point2 p, Vec2 vectors[4], float color[]) {
	int i;
	float pathPoints[10];

	for (i = 0; i < 8; i += 2) {
		pathPoints[i] = p.x + vectors[i / 2].x;
		pathPoints[i + 1] = p.y + vectors[i / 2].y;
	}

	pathPoints[i] = p.x + vectors[0].x;
	pathPoints[i + 1] = p.y + vectors[0].y;

	/*Color rgb interior del polígono*/
	MOMOS::DrawSetStrokeColor(color[0], color[1], color[2], color[3]);

	/*Pinta la misma figura rellena. El último parámetro determina si se muestra el borde*/
	MOMOS::DrawPath(pathPoints, 5);
}


//Shortcut para la matrix de rotación aplicada a un vector
void rotateVec2(Vec2 *v, float angle) {
	float x, y;
	x = (*v).x;
	y = (*v).y;

	(*v).x = x*cos(angle) - y*sin(angle);
	(*v).y = x*sin(angle) + y*cos(angle);
}


void afineTransformFromPoint2(Point2 current, Mat2 m, Point2 origin, Point2 *result) {
	Vec2 v1;

	initVec2FromPoints(&v1, current, origin);
	multMat2Vec2(m, v1, &v1);
	AddPoint2Vec2(origin, v1, &(*result));
}


void drawLine(Point2 p1, Point2 p2) {
	MOMOS::DrawLine(p1.x, p1.y, p2.x, p2.y);
}

void MultMat3(Mat3 m1, Mat3 m2, Mat3 *result) {}
void MultMat3vec3(Mat3 m, Vec3 v, Vec3 *result) {}
void Mat3Scale(Mat3 m1, float n, Mat3 *m2) {}
void Mat3Rotate(Mat3 *m, float angle) {}
void Mat3Translate() {}

/****************** Funciones de prueba de diferentes conceptos ahead ********************/

/*
int rotatingSquare_main() {
Point2 center;
Vec2 vectors[4], vectors2[4];
int i;
float length = 250;

float color[4] = { 255, 255, 255, 255 };
float angle = 1;
Mat2 rotation;


initPoint2(&center, win_width / 2, win_height / 2);


initVec2(&vectors[0], -length, length);
initVec2(&vectors[1], length, length);
initVec2(&vectors[2], length, -length);
initVec2(&vectors[3], -length, -length);


MOMOS::WindowInit(win_width, win_height);

while (MOMOS::WindowIsOpened() && !MOMOS::IsSpecialKeyDown(MOMOS::kSpecialKey_Escape)){

MOMOS::DrawClear(150, 150, 150);

//Asignamos valores a la matriz de rotación
initMat2(&rotation, cos(rads(angle)), -sin(rads(angle)), sin(rads(angle)), cos(rads(angle)));

//rotamos el cuadrado
for (i = 0; i < 4; i++) {
multMat2Vec2(rotation, vectors[i], &vectors2[i]);
//rotateVec2(&vectors[i], 0.05);
}

printSquare(center, vectors, color);

//Copiamos vect2 en vect
for (i = 0; i < 4; i++) {
vectors[i] = vectors2[i];
}

MOMOS::WindowFrame();
}

MOMOS::WindowDestroy();
return 0;
}
int arrow_main() {
Point2 center, mouse;
Vec2 vector, arrow_base;
float arrow_width = 15.0f;

Vec2 v1, v2, v3;
Point2 p1, p2, p3;

//Iniciamos el centro de origen del vector
initPoint2(&center, win_width / 2, win_height / 2);

MOMOS::WindowInit(win_width, win_height);

while (MOMOS::WindowIsOpened() && !MOMOS::IsSpecialKeyDown(MOMOS::kSpecialKey_Escape)){

MOMOS::DrawClear(150, 150, 150);

//Dibujamos un vector desde el centro de la pantalla hasta la posición del cursor
initPoint2(&mouse, MOMOS::MousePositionX(), MOMOS::MousePositionY());
initVec2FromPoints(&vector, center, mouse);
DrawVec2(vector, center);

//Obtenemos el vector base de la flecha de la longitud especificada
perpendicularVec2(vector, &arrow_base);
normalizeVec2(arrow_base, &arrow_base);
scaleVec2(arrow_base, arrow_width, &arrow_base);


//Base derecha de la flecha
normalizeVec2(vector, &v1);
scaleVec2(v1, arrow_width, &v1);
SubstractVec2(vector, v1, &v1);
AddPoint2Vec2(center, v1, &p1);
AddPoint2Vec2(p1, arrow_base, &p2);
DrawVec2(arrow_base, p1);

//Base izquierda de la flecha
scaleVec2(arrow_base, -1, &arrow_base);
AddPoint2Vec2(p1, arrow_base, &p3);
DrawVec2(arrow_base, p1);

//Laterales de la flecha
initVec2FromPoints(&v2, p2, mouse);
DrawVec2(v2, p2);
initVec2FromPoints(&v3, p3, mouse);
DrawVec2(v3, p3);

MOMOS::WindowFrame();
}

MOMOS::WindowDestroy();
return 0;
}
int monigote_main() {
Vec2 vectors[9];
Point2 points[9];
Point2 p;
Vec2 v, v2;
Mat2 rotation, scaling, transform;
float angle = 40;
float scale = 2.1;

int i;

//puntos cuerpo
initPoint2(&points[0], 200, 200);
initPoint2(&points[1], 200, 230);
initPoint2(&points[2], 200, 230);
initPoint2(&points[3], 200, 300);
initPoint2(&points[4], 200, 300);
//puntos cabeza
initPoint2(&points[5], 180, 200);
initPoint2(&points[6], 220, 200);
initPoint2(&points[7], 220, 160);
initPoint2(&points[8], 180, 160);

//vectores cuerpo
initVec2(&vectors[0], 0.0f, 100.0f);
initVec2(&vectors[1], 50.0f, 40.0f);
initVec2(&vectors[2], -50.0f, 40.0f);
initVec2(&vectors[3], 50.0f, 50.0f);
initVec2(&vectors[4], -50.0f, 50.0f);
//vectores cabeza
initVec2(&vectors[5], 40.0f, 0.0f);
initVec2(&vectors[6], 0.0f, -40.0f);
initVec2(&vectors[7], -40.0f, 0.0f);
initVec2(&vectors[8], 0.0f, 40.0f);

MOMOS::WindowInit(win_width, win_height);

while (MOMOS::WindowIsOpened() && !MOMOS::IsSpecialKeyDown(MOMOS::kSpecialKey_Escape)){

MOMOS::DrawClear(150, 150, 150);

//Monigote inicial
for (i = 0; i < 9; i++) {
DrawVec2(vectors[i], points[i]);
}

//Matriz de rotación
initMat2(&rotation, cos(rads(angle)), -sin(rads(angle)), sin(rads(angle)), cos(rads(angle)));
//Matriz de escalado
initMat2(&scaling, scale, 0, 0, scale);
//Combinamos ambas matrices
multMat2(rotation, scaling, &transform);

//Creamos otro monigote a la derecha del inicial
for (i = 0; i < 9; i++) {
initVec2(&v, 450, 0);
AddPoint2Vec2(points[i], v, &p);

multMat2Vec2(transform, vectors[i], &v);
DrawVec2(v, p);
}

MOMOS::WindowFrame();
}

MOMOS::WindowDestroy();
return 0;
}
*/
/*
int planeta_main() {

Point2 center, sq_center;
Mat2 rotation, translation;
float color[4] = { 255, 255, 255, 255 };
Vec2 square[4], vectors[4], vectors2[4], v1;
float length = 20, distance = 200;
float angle_rot = 5, angle_trans = 1;
int i;


//Iniciamos el centro de origen de la translación
initPoint2(&center, win_width / 2, win_height / 2);
//Y el centro de la figura
initPoint2(&sq_center, center.x + 300, center.y);
//Iniciamos el vector que une ambos centros
initVec2FromPoints(&v1, center, sq_center);

//Matrices de rotación
initMat2(&rotation, cos(rads(angle_rot)), -sin(rads(angle_rot)), sin(rads(angle_rot)), cos(rads(angle_rot)));
initMat2(&translation, cos(rads(angle_trans)), -sin(rads(angle_trans)), sin(rads(angle_trans)), cos(rads(angle_trans)));

//Crear vectores para el cuadrado, que determinan sus aristas partiendo de su centro
initVec2(&vectors[0], -length, length);
initVec2(&vectors[1], length, length);
initVec2(&vectors[2], length, -length);
initVec2(&vectors[3], -length, -length);


MOMOS::WindowInit(win_width, win_height);

while (MOMOS::WindowIsOpened() && !MOMOS::IsSpecialKeyDown(MOMOS::kSpecialKey_Escape)){

MOMOS::DrawClear(150, 150, 150);

//recalculamos el vector que une ambos centros, aplicándole una rotación
multMat2Vec2(translation, v1, &v1);
//Calculamos el nuevo centro del cuadrado sumándole v1 al centro de la pantalla
AddPoint2Vec2(center, v1, &sq_center);

//rotamos cada uno de los vectores del cuadrado
for (i = 0; i < 4; i++) {
multMat2Vec2(rotation, vectors[i], &vectors[i]);
}

//Pintamos cosas
printSquare(sq_center, vectors, color);
DrawVec2(v1, center);
MOMOS::WindowFrame();
}

MOMOS::WindowDestroy();
return 0;
}

int planeta2_main() {
Point2 c[4], cp[4];

initPoint2(&c[0], 100, 100);
initPoint2(&c[0], 150, 100);
initPoint2(&c[0], 150, 150);
initPoint2(&c[0], 100, 150);

Mat2 m, ms;
Point2 origin;
float angle = 2;
initMat2(&m, cos(rads(angle)), -sin(rads(angle)), sin(rads(angle)), cos(rads(angle)));
initPoint2(&origin, 100, 100);

Mat2 m2;
Point2 origin2;
float angle2 = 5;
initMat2(&m2, cos(rads(angle2)), -sin(rads(angle2)), sin(rads(angle2)), cos(rads(angle2)));
initPoint2(&origin2, 200, 300);

MOMOS::WindowInit(win_width, win_height);

while (MOMOS::WindowIsOpened() && !MOMOS::IsSpecialKeyDown(MOMOS::kSpecialKey_Escape)){

MOMOS::DrawClear(150, 150, 150);

for (int i = 0; i < 4; i++) {
Point2 tmp;
afineTransformFromPoint2(c[i], m, origin, &tmp);
afineTransformFromPoint2(tmp, m2, origin2, &cp[i]);
}

MOMOS::DrawSetStrokeColor(255, 255, 255, 255);
for (int i = 0; i < 4; i++) {
drawLine(cp[i], cp[(i - 1) % 4]);
}

MOMOS::WindowFrame();
}

MOMOS::WindowDestroy();
return 0;
}

/* El corazón de la bestia */
/*
int MOMOS::main(int argc, char **argv) {

return planeta2_main();
}
*/