in float foo;

void main (void)
{
     vec2 p = gl_TexCoord[0].st;

     //gradients
     vec2 px = dFdx(p);
     vec2 py = dFdy(p);
     
     //chain rule
     float fx = ((2 * p.x) * px.x) - px.y;
     float fy = ((2 * p.x) * py.x) - py.y;
     
     //signed distance
     float sd = ((p.x * p.x) - p.y)/sqrt(fx*fx + fy*fy);

     //linear alpha
     float alpha = 0.5 - sd;
     
     if (foo < 0)
     {
          alpha = 1 - alpha;
     }
     else if (foo == 0)
     {
      	  alpha = 1;
     }

     if (alpha > 1)
     {
          alpha = 1;
     }
     else if (alpha < 0)
     {
          alpha = 0;
     }
     
     if (alpha < 0.5)
     {
          alpha = 0;
     }
     else if (alpha > 0.5)
     {
          alpha = 1;
     }

     gl_FragColor = gl_TexCoord[1];
     gl_FragColor.w = alpha;
}
