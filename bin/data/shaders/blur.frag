#version 120
#extension GL_ARB_draw_buffers : enable
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect tex0;
uniform float blurAmnt;
uniform float blurDist;

void main()
{
    vec4 color = vec4(0);
 	vec2 texCoordVarying = gl_TexCoord[0].xy;
	vec4 mid = texture2DRect(tex0, texCoordVarying);
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurDist, blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(        0, blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2( blurDist, blurDist));

    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurDist, 0.0));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(blurDist, 0.0));
 
    color += 5 * mid;
 
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurDist,-blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(        0,-blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2( blurDist,-blurDist));
 
    color /= 13.0;
 
	gl_FragColor = color;
}