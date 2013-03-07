
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
	GLfloat w;
} vector4;

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
	vector4 a;
	vector4 b;
	vector4 c;
} triangle;
