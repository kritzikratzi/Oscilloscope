#extension GL_EXT_geometry_shader4 : enable  
varying out vec2 texcoord;


//layout(points) in;  
//layout(quads, max_vertices = 4) out;  

uniform float height;  
uniform float width;  

void main() {  
	gl_PositionIn[0];  
	float w = width*1.0;
	// shit? !
	float h = w*16.0/10.0; 
	

	texcoord=vec2(0,0);
	gl_FrontColor = gl_FrontColorIn[0]; 
	gl_Position = gl_PositionIn[0]+vec4(0,0,0,0);
	EmitVertex();  

	texcoord=vec2(0,1);
	gl_FrontColor = gl_FrontColorIn[0]; 
	gl_Position = gl_PositionIn[0]+vec4(0,h,0,0);
	EmitVertex();  


	texcoord=vec2(1,0);
	gl_FrontColor = gl_FrontColorIn[0]; 
	gl_Position = gl_PositionIn[0]+vec4(w,0,0,0);
	EmitVertex();  

	texcoord=vec2(1,1);
	gl_FrontColor = gl_FrontColorIn[0]; 
	gl_Position = gl_PositionIn[0]+vec4(w,h,0,0);
	EmitVertex();  

	// output  
	EndPrimitive();  
}  
