#define BOOL Uint8
#define TRUE 1
#define FALSE 0

#define MAX_VERTEX_COUNT 5000
#define SIZE_OF_VERTEX 3
#define SIZE_OF_COLOR 3
#define SIZE_OF_TEXCOORD 2

#define SNES_SCREEN_WIDTH 512
#define SNES_SCREEN_HEIGHT 448

#define SHOW_GRAPH

#define GLEW_STATIC

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glew.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "structs.h"
#include "pixel.h"

const char* FRAGMENT_SHADER_FILENAME = "QuadraticPS.glsl";
const char* VERTEX_SHADER_FILENAME = "QuadraticVS.glsl";

SDL_Surface* screen;
Uint32 lastTickTime;

SDL_Surface* sprite;

BOOL doneWindow = FALSE;

int vertexCount;
GLfloat* vertexArray = NULL;
GLfloat* colorArray = NULL;
GLfloat* textureArray = NULL;

GLuint myProgObj;

GLuint myFragObj;
GLuint myVertObj;
GLchar* fSource;
GLchar* vSource;

AdjacencyCell adjacencyMatrix[SNES_SCREEN_WIDTH][SNES_SCREEN_HEIGHT];

// this method was found from OpenGL: A Primer by Edward Angel
char* readShaderSource(const char* shaderFile)
{
	struct stat statBuf;
	FILE* fp = fopen(shaderFile, "r");
	char* buf;
	
	stat(shaderFile, &statBuf);
	buf = (char*) malloc((statBuf.st_size + 1) * sizeof(char));
	fread(buf, 1, statBuf.st_size, fp);
	buf[statBuf.st_size] = '\0';
	fclose(fp);
	return buf;
}

void createGraph()
{
	if( SDL_MUSTLOCK( sprite ) )
	{
		//Lock the surface
		SDL_LockSurface( sprite );
	}
	
	//
	
	if( SDL_MUSTLOCK( sprite ) )
	{
		//Lock the surface
		SDL_UnlockSurface( sprite );
	}
}

void pushTriangle(triangle* t)
{
	if (vertexCount >= MAX_VERTEX_COUNT)
	{
		return;
	}

	GLfloat z = (t->type == CONCAVE) ? -1.f : ((t->type == CONVEX) ? 1.f : 0.f);

	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 0] = t->a.x;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 1] = t->a.y;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 2] = z;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 3] = t->b.x;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 4] = t->b.y;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 5] = z;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 6] = t->c.x;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 7] = t->c.y;
	vertexArray[(vertexCount * SIZE_OF_VERTEX) + 8] = z;
	
	colorArray[(vertexCount * SIZE_OF_COLOR) + 0] = t->color.r;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 1] = t->color.g;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 2] = t->color.b;
        colorArray[(vertexCount * SIZE_OF_COLOR) + 3] = t->color.r;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 4] = t->color.g;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 5] = t->color.b;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 6] = t->color.r;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 7] = t->color.g;
	colorArray[(vertexCount * SIZE_OF_COLOR) + 8] = t->color.b;

	textureArray[(vertexCount * SIZE_OF_TEXCOORD) + 0] = 0.0f;
        textureArray[(vertexCount * SIZE_OF_TEXCOORD) + 1] = 0.0f;
	textureArray[(vertexCount * SIZE_OF_TEXCOORD) + 2] = 0.5f;
	textureArray[(vertexCount * SIZE_OF_TEXCOORD) + 3] = 0.0f;
	textureArray[(vertexCount * SIZE_OF_TEXCOORD) + 4] = 1.0f;
	textureArray[(vertexCount * SIZE_OF_TEXCOORD) + 5] = 1.0f;

	vertexCount += 3;
}

BOOL init()
{
	freopen( "CON", "wt", stdout );
	//freopen( "CON", "wt", stderr );

	if ((vertexArray = malloc(sizeof(GLfloat) * MAX_VERTEX_COUNT * SIZE_OF_VERTEX)) == NULL || (colorArray = malloc(sizeof(GLfloat) * MAX_VERTEX_COUNT * SIZE_OF_COLOR)) == NULL || (textureArray = malloc(sizeof(GLfloat) * MAX_VERTEX_COUNT * SIZE_OF_VERTEX)) == NULL)
	{
		perror("error allociating vertex arrays");
		return FALSE;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		perror("error setting up SDL");
		return FALSE;
	}

	if ((screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL)) == NULL)
	{
		perror("error setting video mode");
		return FALSE;
	}
	
	if  ((sprite = IMG_Load("img/yoshi.tif")) == NULL)
	{
		perror("error loading test PNG");
		return FALSE;
	}

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return FALSE;
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	
	myProgObj = glCreateProgram();
	myFragObj = glCreateShader(GL_FRAGMENT_SHADER);
	myVertObj = glCreateShader(GL_VERTEX_SHADER);

	glAttachShader(myProgObj, myFragObj);
	glAttachShader(myProgObj, myVertObj);
	fSource = readShaderSource(FRAGMENT_SHADER_FILENAME);
	vSource = readShaderSource(VERTEX_SHADER_FILENAME);

	glShaderSource(myFragObj, 1, (const GLchar**)&fSource, NULL);
	glCompileShader(myFragObj);
	GLint isCompiledFrag = 0;
	glGetShaderiv(myFragObj, GL_COMPILE_STATUS, &isCompiledFrag);
	if (isCompiledFrag == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(myFragObj, GL_INFO_LOG_LENGTH, &maxLength);
		
		GLchar infoLog[maxLength];
		glGetShaderInfoLog(myFragObj, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Error Compiling Fragment Shader:\n%s\n", infoLog);

		glDeleteShader(myFragObj);
		free(fSource);
		free(vSource);
		
		return FALSE;
	}
	

	glShaderSource(myVertObj, 1, (const GLchar**)&vSource, NULL);
	glCompileShader(myVertObj);
        GLint isCompiledVert = 0;
	glGetShaderiv(myVertObj, GL_COMPILE_STATUS, &isCompiledVert);
	if (isCompiledVert == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(myVertObj, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar infoLog[maxLength];
		glGetShaderInfoLog(myVertObj, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Error Compiling Vertex Shader:\n%s\n", infoLog);

		glDeleteShader(myVertObj);
		free(fSource);
		free(vSource);

		return FALSE;
	}

	glLinkProgram(myProgObj);
	
	glUseProgram(myProgObj);

        glPointSize(8.0f);
	glClearColor(0, 0, 0, 0);
	glViewport(0, 0, 640, 480);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glLoadIdentity();
	
	vertexCount = 0;

	lastTickTime = SDL_GetTicks();

	return TRUE;
}

void deinit()
{
	free(vertexArray);
	free(colorArray);
	free(textureArray);

	free(fSource);
	free(vSource);
	
	SDL_FreeSurface(sprite);

	SDL_Quit();
}

void update(double delta)
{
	int i,j;

	vertexCount = 0;

	triangle t;
	t.type = TRIANGLE;
	
	if( SDL_MUSTLOCK( sprite ) )
	{
		//Lock the surface
		SDL_LockSurface( sprite );
	}

	for (i = 0; i < sprite->w; i++)
	{
		for (j = 0; j < sprite->h; j++)
		{
			Uint8 r, g, b;
			Uint32 currentPixel = get_pixel32(sprite, i, j);
			SDL_GetRGB(currentPixel, sprite->format, &r, &g, &b);
			
			t.color.r = r/255.f;
			t.color.g = g/255.f;
			t.color.b = b/255.f;

			t.a.x = (16 * i);
			t.a.y = (16 * j);
			t.b.x = (16 * i) + 16;
			t.b.y = (16 * j);
			t.c.x = (16 * i) + 16;
			t.c.y = (16 * j) + 16;
			pushTriangle(&t);
			t.a.x = (16 * i);
			t.a.y = (16 * j);
			t.b.x = (16 * i) + 16;
			t.b.y = (16 * j) + 16;
			t.c.x = (16 * i);
			t.c.y = (16 * j) + 16;
			pushTriangle(&t);

		}
	}
	
	if( SDL_MUSTLOCK( sprite ) )
	{
		//Lock the surface
		SDL_UnlockSurface( sprite );
	}
}

void render()
{
	int i,j;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//enable vertex array writing
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//set the array the GPU should use
	glVertexPointer(SIZE_OF_VERTEX, GL_FLOAT, 0, vertexArray);
	glColorPointer(SIZE_OF_COLOR, GL_FLOAT, 0, colorArray);
	glTexCoordPointer(SIZE_OF_TEXCOORD, GL_FLOAT, 0, textureArray);

	//draw the first 8 elements as a triangle
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	
	//disable vertex array writing
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glBegin(GL_POINTS);
	glColor3f(1.0f, 0.0f, 1.0f);
	for (i = 0; i < sprite->w; i++)
	{
		for (j = 0; j < sprite->h; j++)
		{
			glVertex2i(i * 16 + 8, j * 16 + 8);
		}
	}
	glEnd();
	
	/*
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	for (i = 0; i < sprite->w; i++)
	{
		for (j = 0; j < sprite->h; j++)
		{
			//getAdjacentNodes(i, j, &adjNum, adjW, adjH);

			for (q = 0; q < adjNum; q++)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(adjW[q] * 16 + 8, adjH[q] * 16 + 8);
			}
		}
	}
	glEnd();
	*/

	SDL_GL_SwapBuffers();
}

int main(int argc, char* argv[])
{
	if (init() != TRUE)
	{
		perror("error initalizing SDL");
		
		deinit();

		return 1;
	}
	
	createGraph();
	
	while (!doneWindow)
	{
		Uint32 currentTickTime = SDL_GetTicks();

		update((double)(currentTickTime - lastTickTime));
		render();
		
		lastTickTime = currentTickTime;
		
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				doneWindow = TRUE;
			}
		}
	}
	
	deinit();

	return 0;
}
