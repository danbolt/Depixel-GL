
#include <SDL/SDL.h>

#include "structs.h"

#define W_R 0.299f
#define W_B 0.114f
#define W_G 0.587f
#define U_MAX 0.436f
#define V_MAX 0.615f

void RGB2YUV(color4* in, yuv3* out)
{
	if (in == NULL || out == NULL)
	{
		return;
	}
	
	out->y = (W_R * in->r) + (W_G * in->g) + (W_B * in->b);
	out->u = U_MAX * ((in->b - out->y) / (1.f - W_B));
	out->v = V_MAX * ((in->r - out->y) / (1.f - W_R));
}

BOOL arePixelColorsAlike(color4 node1, color4 node2)
{
	const GLfloat yDiffMin = 48.f/255.f;
	const GLfloat uDiffMin = 7./255.f;
	const GLfloat vDiffMin = 6./255.f;

	yuv3 node1_YUV;
	yuv3 node2_YUV;
	
	RGB2YUV(&node1, &node1_YUV);
	RGB2YUV(&node2, &node2_YUV);

	if (node1_YUV.y - node2_YUV.y > yDiffMin || node1_YUV.u - node2_YUV.u > uDiffMin || node1_YUV.v - node2_YUV.v > vDiffMin)
	{
		return FALSE;
	}
	
	return TRUE;
}
