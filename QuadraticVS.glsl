/* passthrough vertex shader */

uniform float scale;
uniform vec2 translate;

out float foo;

void main()
{
   gl_Position = ftransform();
   
   gl_Position.x += translate.x;
   gl_Position.y += translate.y;
   gl_Position.x *= scale;
   gl_Position.y *= scale;

   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_TexCoord[1] = gl_Color;
   
   foo = gl_Position.z;
}