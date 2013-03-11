
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
