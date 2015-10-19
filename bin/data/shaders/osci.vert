#version 110
uniform mat3 uMatrix;
void main()
{
	gl_FrontColor = gl_Color;
	gl_Position = gl_Vertex;
}
