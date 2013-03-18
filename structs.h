
#define GLEW_STATIC
#include "glew.h"

#define BOOL Uint8
#define TRUE 1
#define FALSE 0

typedef enum
{
	TRIANGLE = 0,
	CONVEX = 1,
	CONCAVE = -1
} triangleType;

typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vector3;

typedef struct
{
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
} color4;

typedef struct
{
	GLfloat y;
	GLfloat u;
	GLfloat v;
} yuv3;

typedef struct
{
	GLfloat type;
	color4 color;
	vector3 a;
	vector3 b;
	vector3 c;
} triangle;

typedef struct
{
	color4 color;
	BOOL N;
	BOOL NE;
	BOOL E;
	BOOL SE;
	BOOL S;
	BOOL SW;
	BOOL W;
	BOOL NW;
	int sparse;
} AdjacencyCell;

void RGB2YUV(color4* in, yuv3* out);

BOOL arePixelColorsAlike(color4 node1, color4 node2);
