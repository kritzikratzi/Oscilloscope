#version 150

in vec4 in_pos;
// we're not using this here.
// uniform mat4 modelViewProjectionMatrix;

out Vertex
{
	vec4 pos;
	vec4 color;
} vertex;


void main()
{
	vertex.pos = in_pos;
	vertex.color =  vec4(1.0, 1.0, 0.0, 1.0);
}
