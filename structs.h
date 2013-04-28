#include <stdint.h>

#define GLEW_STATIC
#include "glew.h"

#define DAN_BOOL Uint8
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
	DAN_BOOL N;
	DAN_BOOL NE;
	DAN_BOOL E;
	DAN_BOOL SE;
	DAN_BOOL S;
	DAN_BOOL SW;
	DAN_BOOL W;
	DAN_BOOL NW;
	int sparse;
	int adjacencyByte;
} AdjacencyCell;

void RGB2YUV(color4* in, yuv3* out);

DAN_BOOL arePixelColorsAlike(color4 node1, color4 node2);
