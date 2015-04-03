#define DrawText DrawText
#define M_PI 3.14159265358979323846;

typedef struct {
	float x;
	float y;
} Point2, Vec2;

typedef float Mat2[2][2];


void initVec2(Vec2 *v, float x, float y);
void initPoint2(Point2 *p, float x, float y);
void initMat2(Mat2 *m, float w, float x, float y, float z);
void intMat2FromBasis(Mat2 m, Vec2 a1, Vec2 a2);
void initVec2FromPoints(Vec2 *v, Point2 a, Point2 b);
float Dot(Vec2 v1, Vec2 v2);
void AddPoint2Vec2(Point2 p1, Vec2 v1, Point2 *p2);
void AddVec2(Vec2 v1, Vec2 v2, Vec2 *v3);
void SubstractVec2(Vec2 v1, Vec2 v2, Vec2 *v3);
void scaleVec2(Vec2 v1, float n, Vec2 *v2);
void perpendicularVec2(Vec2 v1, Vec2 *v2);
float LenghtVec2(Vec2 v1);
void normalizeVec2(Vec2 v1, Vec2 *v2);
void projectionVec2(Vec2 v1, Vec2 v2, Vec2 *v3);
void combinePoint2(Point2 v1, Point2 v2, float f, Point2*);
void multMat2(Mat2 m1, Mat2 m2, Mat2 *m3);
void multMat2Vec2(Mat2 m1, Vec2 v1, Vec2 *v2);
void addMat2(Mat2 m1, Mat2 m2, Mat2 *m3);
void scaleMat2(Mat2 m1, float n, Mat2 *m2);
float rads(float degrees);
void DrawVec2(Vec2 v, Point2 p);

/* Dibuja un cuadrado con su esquina superior izquierda en las coordenadas indicadas */
void printSquare(Point2 p, Vec2 vectors[4], float color[]);

//Shortcut para la matrix de rotación aplicada a un vector
void rotateVec2(Vec2 *v, float angle);

void afineTransformFromPoint2(Point2 current, Mat2 m, Point2 origin, Point2 *result);

void drawLine(Point2 p1, Point2 p2);