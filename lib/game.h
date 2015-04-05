/*
El clásico juego Asteroids, con registro y control de usuarios y puntuaciones.

Marcos Vázquez Rey
ESAT 2015
*/


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


void initPlayers();
/* Dibuja una figura con su esquina superior izquierda en las coordenadas indicadas */
void printFigure(tFigura figura);
/* Rota una figura los grados indicados */
void rotateFigure(tFigura(*figura), float degrees);

/* Recalcula las coordenadas de los vértices del polígono, pasándole las nuevas coordenadas del nuevo centro
Además, aplica la función de escalado indicada por el parámetro scale */
void recalculateCoordinates(tFigura(*figura), Point2 newpos, float scale);

/* Dibuja la animación del motor en la parte trasera de la nave */
void printThruster(tShip(*ship));

/*Comprueba si un punto está fuera del borde la pantalla y transforma p2 en el punto correcto.
Devuelve 1 si el punto se encontraba fuera de los límites, o 0 en caso contrario*/
int checkScreenBorders(Point2 p1, Point2 *p2);

/* Escucha la pulsación de las teclas AWSD y mueve la nave según su aceleración almacenada */
void moveShip(tShip(*ship));

/* Calcula el centroide y devuelve los puntos x e y pertenecientes al centro del polígono */
void getFigureCenter(tFigura figura, Point2 *p);

/* Crea la figura de la nave, utilizada tanto para el control del jugador como para indicar el número de vidas*/
tFigura createShipFigure(int width, int height, Point2 center);

/* Crea la nave que maneja el jugador*/
tShip createShip(int width, int height);

/* Ejecuta un rayCast sobre un polígono y devuelve si los puntos indicados se encuentran dentro del area del mismo */
int rayCast(tFigura figura, Point2 p);

/* Crea un disparo desde la nave */
void shoot(tShip *ship, tShot shots[50], int *num_shots);

/* Crea un disparo desde el ovni */
void shoot(tShip *ship, tShot shots[50], int *num_shots, tOvni *ovni);

//Elimina un disparo del vector de disparos
void removeShot(tShot shots[50], int i, int *num_shots);

/* Desplaza los disparos y los hace desaparecer si han recorrido la distancia máxima */
void moveShots(tShot shots[50], int *num_shots);

/* Crea un asteroide */
tAsteroid createAsteroid(int type, int size, Point2 origin);

/* Crea un ovni */
tOvni createOvni(int type);

/* Divide o elimina un asteroide */
void breakAsteroid(tAsteroid asteroids[50], int i, int *num_asteroids, int scores);

/* Itera sobre los asteroides y disparos, comprobando si alguno de ellos ha impactado */
void checkShotsAsteroidsHit(tAsteroid asteroids[50], int *num_asteroids, tShot shots[5000], int *num_shots);


/* Marca la nave como destruída e inicia los vectores del efecto gráfico de destrucción*/
void destroyShip(tShip *ship);

void destroyOvni(tOvni *ovni);

/* Dibuja y avanza la animaciónd e destrucción de la nave. Devuelve 1 si la animación ha terminado*/
void printShipDestruction(tShip *ship);

void checkShipAsteroidsHit(tAsteroid asteroids[50], int *num_asteroids, tShip *ship);

void checkAsteroidsOvniHit(tAsteroid asteroids[50], int *num_asteroids, tOvni *ovni);

void checkShotsOvniHit(tShot shots[5000], int *num_shots, tOvni *ovni);

void checkShotsShipHit(tShot shots[5000], int *num_shots, tShip *ship);

void checkShipOvniHit(tShip *ship, tOvni *ovni);

/* Itera sobre los elementos que pueden colisionar entre ellos y hace las comprobaciones pertinentes */
void checkHits(tAsteroid asteroids[50], int *num_asteroids, tShot shots[5000], int *num_shots, tShip *ship, tOvni *ovni, int check_ovni);

/* Dibuja los disparos */
void printShots(tShot shots[50], int num_shots);

/* Crea todos los asteroides */
void createAsteroids(tAsteroid asteroids[500], int *num_asteroids);

/* Mueve los asteroides */
void moveAsteroids(tAsteroid asteroids[50], int num_asteroids);

void moveOvni(tOvni *ovni);


/* Dibuja cada uno de los asteroides */
void printAsteroids(tAsteroid asteroids[50], int num_asteroids);

void initText();

void printInfo();

/* Comprueba si debe procesarse el siguiente frame y actualiza los FPS */
int showNextFrame(float diff, float *fps);

int game();