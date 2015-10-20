#version 150
layout(lines_adjacency) in;
layout(triangle_strip, max_vertices=200) out;

uniform mat4 modelViewProjectionMatrix;
uniform float uSize;


in Vertex
{
	vec4 pos;
} vertex[];


out vec4 vertex_color;
out vec4 uvl;

void out2( vec2 v ){
	gl_Position = modelViewProjectionMatrix*vec4(v,0,1);
	vertex_color = vec4(1);
	EmitVertex();
}
void main()
{
	vec2 a = vertex[0].pos.xy;
	vec2 b = vertex[1].pos.xy;
	vec2 c = vertex[2].pos.xy;
	vec2 d = vertex[3].pos.xy;

	vec2 ba = b-a;
	vec2 cb = c-b;
	vec2 cd = d-c;
	
	float len = length(cb);
	uvl.z = len;
	
	if( len == 0 ){
		// fuck it. we don't need this!
	}
	else{
		vec2 dir = cb/len;
		vec2 normal = vec2(-dir.y,dir.x);
		float uInvert = 1;
		vec2 b1 = (b+(-dir-normal)*uSize)*uInvert;
		vec2 b2 = (b+(-dir+normal)*uSize)*uInvert;
		vec2 c1 = (c+(+dir-normal)*uSize)*uInvert;
		vec2 c2 = (c+(+dir+normal)*uSize)*uInvert;

		uvl.xyw = vec3(-uSize,-uSize,1.5);
		out2( b1 );
		uvl.xyw = vec3(-uSize,uSize,0.5);
		out2( b2 );
		uvl.xyw = vec3(uSize+len,-uSize,1.5);
		out2( c1 );
		uvl.xyw = vec3(uSize+len,uSize,0.5);
		out2( c2 );
	}
}
