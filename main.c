#define BOOL Uint8
#define TRUE 1
#define FALSE 0

#define MAX_VERTEX_COUNT 5000
#define SIZE_OF_VERTEX 3
#define SIZE_OF_COLOR 3
#define SIZE_OF_TEXCOORD 2

#define SNES_SCREEN_WIDTH 512
#define SNES_SCREEN_HEIGHT 448

//#define SHOW_GRAPH

#define GLEW_STATIC

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <assert.h>

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

int edges[MAX_VERTEX_COUNT * 4];
int edgeCount = 0;

void pushEdge(int x1, int y1, int x2, int y2)
{
	if (edgeCount >= MAX_VERTEX_COUNT * 4)
	{
		return;
	}
	
	edges[edgeCount + 0] = x1;
	edges[edgeCount + 1] = y1;
	edges[edgeCount + 2] = x2;
	edges[edgeCount + 3] = y2;

	edgeCount += 4;
}

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

void setCellAdjacency(int xa, int ya, int xb, int yb, BOOL value)
{
	if (xa - xb == 1)
	{
		if (ya - yb == 1)
		{
			adjacencyMatrix[xa][ya].NW = value;
			adjacencyMatrix[xb][yb].SE = value;
			return;
		}
		else if (ya - yb == -1)
		{
			adjacencyMatrix[xa][ya].SW = value;
			adjacencyMatrix[xb][yb].NE = value;
			return;
		}
		else if (ya - yb == 0)
		{
			adjacencyMatrix[xa][ya].W = value;
			adjacencyMatrix[xb][yb].E = value;
			return;
		}
	}
	else if (xa - xb == -1)
	{
		if (ya - yb == 1)
		{
			adjacencyMatrix[xa][ya].NE = value;
			adjacencyMatrix[xb][yb].SW = value;
			return;
		}
		else if (ya - yb == -1)
		{
			adjacencyMatrix[xa][ya].SE = value;
			adjacencyMatrix[xb][yb].NW = value;
			return;
		}
		else if (ya - yb == 0)
		{
			adjacencyMatrix[xa][ya].E = value;
			adjacencyMatrix[xb][yb].W = value;
			return;
		}
	}
	else if (xa - xb == 0)
	{
		if (ya - yb == 1)
		{
			adjacencyMatrix[xa][ya].N = value;
			adjacencyMatrix[xb][yb].S = value;
			return;
		}
		else if (ya - yb == -1)
		{
			adjacencyMatrix[xa][ya].S = value;
			adjacencyMatrix[xb][yb].N = value;
			return;
		}
	}
}

BOOL getCellAdjacency(int xa, int ya, int xb, int yb)
{
	if (xa - xb == 1)
	{
		if (ya - yb == 1)
		{
			if (adjacencyMatrix[xa][ya].NW && adjacencyMatrix[xb][yb].SE)
			{
				return TRUE;
			}
		}
		else if (ya - yb == -1)
		{
			if (adjacencyMatrix[xa][ya].SW && adjacencyMatrix[xb][yb].NE)
			{
				return TRUE;
			}
		}
		else if (ya - yb == 0)
		{
			if (adjacencyMatrix[xa][ya].W && adjacencyMatrix[xb][yb].E)
			{
				return TRUE;
			}
		}
	}
	else if (xa - xb == -1)
	{
		if (ya - yb == 1)
		{
			if (adjacencyMatrix[xa][ya].NE && adjacencyMatrix[xb][yb].SW)
			{
				return TRUE;
			}
		}
		else if (ya - yb == -1)
		{
			if (adjacencyMatrix[xa][ya].SE && adjacencyMatrix[xb][yb].NW)
			{
				return TRUE;
			}
		}
		else if (ya - yb == 0)
		{
			if (adjacencyMatrix[xa][ya].E && adjacencyMatrix[xb][yb].W)
			{
				return TRUE;
			}
		}
	}
	else if (xa - xb == 0)
	{
		if (ya - yb == 1)
		{
			if (adjacencyMatrix[xa][ya].N && adjacencyMatrix[xb][yb].S)
			{
				return TRUE;
			}
		}
		else if (ya - yb == -1)
		{
			if (adjacencyMatrix[xa][ya].S && adjacencyMatrix[xb][yb].N)
			{
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//this is a wrapper method for getCellAdjacency(); it's written to interface with Johannes Kopf's weird if/else code
BOOL buddy(int x, int y, int dir)
{
	switch (dir)
	{
		case 0:
		return getCellAdjacency(x, y, x-1, y-1);
		case 1:
		return getCellAdjacency(x, y, x, y-1);
		case 2:
		return getCellAdjacency(x, y, x+1, y-1);
		case 3:
		return getCellAdjacency(x, y, x+1, y);
		case 4:
		return getCellAdjacency(x, y, x+1, y+1);
		case 5:
		return getCellAdjacency(x, y, x, y+1);
		case 6:
		return getCellAdjacency(x, y, x-1, y+1);
		case 7:
		return getCellAdjacency(x, y, x-1, y);
		default:
		return FALSE;
	}
}

int countCellNeighbours(int x, int y)
{
	AdjacencyCell cell = adjacencyMatrix[x][y];
	int neighbours = 0;
	
	if (cell.N) { neighbours++; }
	if (cell.NE) { neighbours++; }
	if (cell.NW) { neighbours++; }
	if (cell.S) { neighbours++; }
	if (cell.SE) { neighbours++; }
	if (cell.SW) { neighbours++; }
	if (cell.E) { neighbours++; }
	if (cell.W) { neighbours++; }
	
	return neighbours;
}

// team A is the edge with the negative slope
// team B is the edge with the positive slope 

int recurseCurveLength(int x, int y, int prevX, int prevY, int origX, int origY)
{
	//assert(x != -1 && y != -1);

	if (x == origX && y == origY)
	{
		//we've traversed a loop
		return 0;
	}

	int neighbours = countCellNeighbours(x, y);
	
	if (neighbours == 1)
	{
		//end of the road
		return 1;
	}
	else if (neighbours > 2)
	{
		//this is not a curve, it is now a junction
		return 1;
	}
	else if (neighbours == 2)
	{
		AdjacencyCell cell = adjacencyMatrix[x][y];
		
		int nextX = -1;
		int nextY = -1;
		
		if (cell.N && prevX != x)
		{
			nextX = x;
			nextY = y - 1;
		}
		else if (cell.NE && prevY != y - 1 && prevX != x + 1)
		{
			nextX = x + 1;
			nextY = y - 1;
		}
		else if (cell.E && prevY != y && prevX != x + 1)
		{
			nextX = x + 1;
			nextY = y;
		}
		else if (cell.SE && prevY != y + 1 && prevX != x + 1)
		{
			nextX = x + 1;
			nextY = y + 1;
		}
		else if (cell.S && prevY != y + 1 && prevX != x)
		{
			nextX = x;
			nextY = y + 1;
		}
		else if (cell.SW && prevY != y + 1 && prevX != x - 1)
		{
			nextX = x - 1;
			nextY = y + 1;
		}
		else if (cell.W && prevY != y && prevX != x - 1)
		{
			nextX = x - 1;
			nextY = y;
		}
		else if (cell.NW && prevY != y - 1 && prevX != x - 1)
		{
			nextX = x - 1;
			nextY = y - 1;
		}

		return recurseCurveLength(nextX, nextY, x, y, origX, origY) + 1;
	}
	
	//should not get here!
	//assert(neighbours > 0);
	return 0;
}

int curvesHeuristic(int x, int y)
{
	int teamACurveLength = 0;
	int teamBCurveLength = 0;
	
	teamACurveLength = recurseCurveLength(x, y, x+1, y+1, x+1, y+1) + recurseCurveLength(x+1, y+1, x, y, x, y);
	teamBCurveLength = recurseCurveLength(x, y+1, x+1, y, x+1, y) + recurseCurveLength(x+1, y, x, y+1, x, y+1);

	return teamACurveLength - teamBCurveLength;
}

int recurseSparse(int region[8][8], int x, int y, int team, int origX, int origY)
{
	int sum = 0;
	
	int regionX = x - origX + 3;
	int regionY = y - origY + 3;

	if (region[regionX][regionY] != 0)
	{
		return 0;
	}
	
	region[regionX][regionY] = team;
	
	AdjacencyCell cell = adjacencyMatrix[x][y];
	
	if (cell.N && regionY != 0) { sum += recurseSparse(region, x, y-1, team, origX, origY); };
	if (cell.NE && regionY != 0 && regionX != 7) { sum += recurseSparse(region, x+1, y-1, team, origX, origY); };
	if (cell.NW && regionY != 0 && regionX != 0) { sum += recurseSparse(region, x-1, y-1, team, origX, origY); };
	if (cell.S && regionY != 7) {sum += recurseSparse(region, x, y+1, team, origX, origY); };
	if (cell.SE && regionY != 7 && regionX != 7) {sum += recurseSparse(region, x+1, y+1, team, origX, origY); };
	if (cell.SW && regionY != 7 && regionX != 0) {sum += recurseSparse(region, x-1, y+1, team, origX, origY); };
	if (cell.W && regionX != 0) { sum += recurseSparse(region, x-1, y, team, origX, origY); };
	if (cell.E && regionX != 7) { sum += recurseSparse(region, x+1, y, team, origX, origY); };
	
	return sum + 1;
}

int sparsePixelsHeuristic(int x, int y)
{
	int i,j;
	int teamASparseCount = 0;
	int teamBSparseCount = 0;
	int region[8][8]; //1 indicates team A, -1 indicates team B

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			region[i][j] = 0;
		}
	}
	
	teamASparseCount = recurseSparse(region, x, y, 1, x, y) + recurseSparse(region, x+1, y+1, 1, x, y);
	teamBSparseCount = recurseSparse(region, x, y+1, -1, x, y) + recurseSparse(region, x+1, y, -1, x, y);

	return teamBSparseCount - teamASparseCount; //the difference is reversed so the more sparse component recieves the weight
}

int islandsHeuristic(int x, int y)
{
	int teamAIslands = 0;
	int teamBIslands = 0;
	
	if (countCellNeighbours(x, y) == 1)
	{
		teamAIslands += 1;
	}
	if (countCellNeighbours(x + 1, y + 1) == 1)
	{
		teamAIslands += 1;
	}
	
	if (countCellNeighbours(x, y+1) == 1)
	{
		teamBIslands += 1;
	}
	if (countCellNeighbours(x + 1, y) == 1)
	{
		teamBIslands += 1;
	}
	
	if (teamAIslands > teamBIslands)
	{
		return 5;
	}
	else if (teamBIslands > teamAIslands)
	{
		return -5;
	}
	else
	{
		return 0;
	}
}

void weighCrossHeuristics(int x, int y)
{
	if (x == 1 && y == 11)
	{
		fprintf(stdout, "status report:\n");
		fprintf(stdout, "curves heuristic: %d\n", curvesHeuristic(x, y));
		fprintf(stdout, "sparse heuristic: %d\n", sparsePixelsHeuristic(x, y));
		fprintf(stdout, "islands heuristic: %d\n", islandsHeuristic(x, y));
	}

	int weight = curvesHeuristic(x, y) + sparsePixelsHeuristic(x, y) + islandsHeuristic(x, y);
	
	if (weight < 0)
	{
		setCellAdjacency(x, y, x+1, y+1, FALSE);
	}
	else if (weight > 0)
	{
		setCellAdjacency(x+1, y, x, y+1, FALSE);
	}
	else
	{
		setCellAdjacency(x, y, x+1, y+1, FALSE);
		setCellAdjacency(x+1, y, x, y+1, FALSE);
	}
}

void createGraph()
{
	int i,j;
	int x,y;

	// make the graph fully-connected for now
	{
		if( SDL_MUSTLOCK( sprite ) )
		{
			SDL_LockSurface( sprite );
		}
		
		for (i = 0; i < sprite->w; i++)
		{
			for (j = 0; j < sprite->h; j++)
			{
				Uint8 r, g, b;
				Uint32 pixel = get_pixel32(sprite, i, j);
				SDL_GetRGB(pixel, sprite->format, &r, &g, &b);
	
				adjacencyMatrix[i][j].color.r = r/255.f;
				adjacencyMatrix[i][j].color.g = g/255.f;
				adjacencyMatrix[i][j].color.b = b/255.f;
				
				adjacencyMatrix[i][j].N = TRUE;
				adjacencyMatrix[i][j].NE = TRUE;
				adjacencyMatrix[i][j].NW = TRUE;
				adjacencyMatrix[i][j].S = TRUE;
				adjacencyMatrix[i][j].SE = TRUE;
				adjacencyMatrix[i][j].SW = TRUE;
				adjacencyMatrix[i][j].E = TRUE;
				adjacencyMatrix[i][j].W = TRUE;
				
				if (i == 0)
				{
					adjacencyMatrix[i][j].W = FALSE;
					adjacencyMatrix[i][j].NW = FALSE;
					adjacencyMatrix[i][j].SW = FALSE;
				}
				if (j == 0)
				{
					adjacencyMatrix[i][j].N = FALSE;
					adjacencyMatrix[i][j].NW = FALSE;
					adjacencyMatrix[i][j].NE = FALSE;
				}
	
				if (i >= sprite->w - 1)
				{
					adjacencyMatrix[i][j].E = FALSE;
					adjacencyMatrix[i][j].NE = FALSE;
					adjacencyMatrix[i][j].SE = FALSE;
				}
				if (j >= sprite->h - 1)
				{
					adjacencyMatrix[i][j].S = FALSE;
					adjacencyMatrix[i][j].SW = FALSE;
					adjacencyMatrix[i][j].SE = FALSE;
				}
			}
		}
		
		if( SDL_MUSTLOCK( sprite ) )
		{
			SDL_UnlockSurface( sprite );
		}
	}
	
	//remove all diff-coloured adjacencies
	for (i = 0; i < sprite->w; i++)
	{
		for (j = 0; j < sprite->h; j++)
		{
			int p, q;
			
			for (p = i - 1; p <= i + 1; p++)
			{
				for (q = j - 1; q <= j + 1; q++)
				{
					if (i == p && j == q)
					{
						continue;
					}
					
					if (!arePixelColorsAlike(adjacencyMatrix[i][j].color, adjacencyMatrix[p][q].color))
					{
						setCellAdjacency(i, j, p, q, FALSE);
					}
				}
			}
		}
	}
	
	//remove all diagonals of fully connected 2x2s
	for (i = 0; i < sprite->w - 1; i++)
	{
		for (j = 0; j < sprite->h - 1; j++)
		{
			if (getCellAdjacency(i, j, i+1, j) && getCellAdjacency(i, j+1, i+1, j+1))
			{
				if (getCellAdjacency(i, j, i, j+1) && getCellAdjacency(i + 1, j, i+ 1, j + 1))
				{
					if (getCellAdjacency(i, j, i+1, j+1) && getCellAdjacency(i, j+1, i+1, j))
					{
						//this 2x2 square is fully connected
						setCellAdjacency(i, j, i+1, j+1, FALSE);
						setCellAdjacency(i, j+1, i+1, j, FALSE);
					}
				}
			}
		}
	}
	
	//use heuristics to resolve remaining crossings
	for (i = 0; i < sprite->w - 1; i++)
	{
		for (j = 0; j < sprite->h - 1; j++)
		{
			if (getCellAdjacency(i, j, i+1, j+1) && getCellAdjacency(i, j+1, i+1, j))
			{
				weighCrossHeuristics(i, j);
			}
		}
	}

	for (y = 0; y < (sprite->h); y++)
	{
		for (x = 0; x < (sprite->w); x++)
		{
			//center
			//pushEdge(x*16 + 8, y*16, x*16 + 16, y*16 + 8);
			//pushEdge(x*16 + 16, y*16 + 8, x*16 + 8, y*16 + 16);
			//pushEdge(x*16 + 8, y*16 + 16, x*16, y*16 + 8);
			//pushEdge(x*16, y*16 + 8, x*16 + 8, y*16);
			
			//bottom left corner
			if (x == 0 || !buddy(x-1, y, 4))
			{
				pushEdge(x*16, y*16 + 8, x*16, y*16 + 16);
				pushEdge(x*16, y*16 + 16, x*16 + 8, y*16 + 16);
			}
			if (buddy(x, y, 6) && !buddy(x-1, y, 4))
			{
				pushEdge(x*16, y*16 + 8, x*16 - 8, y*16 + 16);
				pushEdge(x*16 + 8, y*16 + 16, x*16, y*16 + 24);
			}

			//bottom right corner
			if (x == (sprite->w - 1) || !buddy(x+1, y, 6))
			{
				pushEdge(x*16 + 16, y*16 + 8, x*16 + 16, y*16 + 16);
				pushEdge(x*16 + 8, y*16 + 16, x*16 + 16, y*16 + 16);
			}
			if (buddy(x, y, 4) && !buddy(x+1, y, 6))
			{
				pushEdge(x*16 + 16, y*16 + 8, x*16 + 24, y*16 + 16);
				pushEdge(x*16 + 8, y*16 + 16, x*16 + 16, y*16 + 24);
			}

			//top left corner
			if (x == 0 || !buddy(x-1, y, 2))
			{
				pushEdge(x*16, y*16, x*16 + 8, y*16);
				pushEdge(x*16, y*16, x*16, y*16 + 8);
			}
			
			//top left corner
			if (x == 0 || !buddy(x+1, y, 0))
			{
				pushEdge(x*16 + 16, y*16, x*16 + 8, y*16);
				pushEdge(x*16 + 16, y*16, x*16 + 16, y*16 + 8);
			}
		}
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
	//freopen( "CON", "wt", stdout );
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
	
	SDL_WM_SetCaption( "Depixel-GL", NULL );

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

	for (i = 0; i < sprite->w; i++)
	{
		for (j = 0; j < sprite->h; j++)
		{
			t.color.r = adjacencyMatrix[i][j].color.r;
			t.color.g = adjacencyMatrix[i][j].color.g;
			t.color.b = adjacencyMatrix[i][j].color.b;

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

#ifdef SHOW_GRAPH
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

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	for (i = 0; i < sprite->w; i++)
	{
		for (j = 0; j < sprite->h; j++)
		{
			if (adjacencyMatrix[i][j].N)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8, j * 16 + 8 - 16);
			}
			
			if (adjacencyMatrix[i][j].NE)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8 + 16, j * 16 + 8 - 16);
			}
			
			if (adjacencyMatrix[i][j].NW)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8 - 16, j * 16 + 8 - 16);
			}

			if (adjacencyMatrix[i][j].S)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8, j * 16 + 8 + 16);
			}
			
			if (adjacencyMatrix[i][j].SE)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8 + 16, j * 16 + 8 + 16);
			}
			
			if (adjacencyMatrix[i][j].SW)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8 - 16, j * 16 + 8 + 16);
			}
			
			if (adjacencyMatrix[i][j].W)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8 - 16, j * 16 + 8);
			}
			
			if (adjacencyMatrix[i][j].E)
			{
				glVertex2i(i * 16 + 8, j * 16 + 8);
				glVertex2i(i * 16 + 8 + 16, j * 16 + 8);
			}
		}
	}
	glEnd();
	
#endif

	glBegin(GL_LINES);
	glColor3f(0.56f, 0.56f, 0.0f);
	for (i = 0; i < edgeCount; i+= 2)
	{
		glVertex2i(edges[i], edges[i+1]);
	}
	glEnd();

	SDL_GL_SwapBuffers();
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		fprintf(stdout, "Usage: depixel-gl <tif file>\n");
		return 0;
	}

	if (init() != TRUE)
	{
		perror("error initalizing SDL");
		
		deinit();

		return 1;
	}

	if  ((sprite = IMG_Load(argv[1])) == NULL)
	{
		perror("error loading test PNG");
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
