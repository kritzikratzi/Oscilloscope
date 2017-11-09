#version 110
uniform mat4 uMatrix;
varying vec4 color;
varying float brightness;

void main()
{
	gl_Position = uMatrix*vec4(gl_Vertex.xy,0,gl_Vertex.w);
	brightness = min(max(0.0,gl_Vertex.z),1.0);
	
	color = gl_Color; 
}