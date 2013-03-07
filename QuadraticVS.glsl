/* passthrough vertex shader */

out float foo;

void main()
{
   gl_Position = ftransform();
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_TexCoord[1] = gl_Color;
   
   foo = gl_Position.z;
}