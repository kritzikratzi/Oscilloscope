#version 120
#extension GL_ARB_draw_buffers : enable
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect tex0;
uniform float blurDist;
uniform float blurDist;

void main()
{
    vec4 color;
 	vec2 texCoordVarying = gl_TexCoord[0].xy;
	float weight = (10-9*blurAmnt);
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurDist, blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(        0, blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2( blurDist, blurDist));

    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurDist, 0.0));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(blurDist, 0.0));
 
    color += weight * texture2DRect(tex0, texCoordVarying);
 
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(-blurDist,-blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(        0,-blurDist));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2( blurDist,-blurDist));
 
    color /= (8+weight);
 
    gl_FragColor = color;
}