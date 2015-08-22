#version 120
 
uniform sampler2DRect tex0;
uniform float blurAmnt;
 
void main()
{
    vec4 color;
 	vec2 texCoordVarying = gl_TexCoord[0].st; 
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurAmnt, blurAmnt));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(        0, blurAmnt));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2( blurAmnt, blurAmnt));

    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurAmnt, 0.0));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt, 0.0));
 
    color += 5.0 * texture2DRect(tex0, texCoordVarying);
 
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurAmnt,-blurAmnt));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(        0,-blurAmnt));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2( blurAmnt,-blurAmnt));
 
    color /= 13.0;
 
    gl_FragColor = color;
	gl_FragColor.g = pow(gl_FragColor.g,1.0);
}