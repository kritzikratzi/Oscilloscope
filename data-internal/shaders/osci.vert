#version 110
uniform mat4 uMatrix;
varying vec4 color;

void main()
{
	gl_Position = uMatrix*gl_Vertex;
	color = gl_Color; 
}