
#define GLEW_STATIC
#include "glew.h"

typedef enum
{
	TRIANGLE,
	CONVEX,
	CONCAVE
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
	triangleType type;
	color4 color;
	vector3 a;
	vector3 b;
	vector3 c;
} triangle;
