#version 120  

uniform sampler2DRect tex;
varying vec2 texcoord;

void main(void)  
{  
	vec4 col = texture2DRect(tex, vec2(texcoord.x*20,texcoord.y*20)); 
    gl_FragColor = gl_Color*col;
	gl_FragColor.a *= 10;
//	gl_FragColor = vec4(texcoord.x,texcoord.y,0,1);
//	gl_FragColor = gl_Color; 
}  
