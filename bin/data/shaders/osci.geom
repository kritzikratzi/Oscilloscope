#version 120
#extension GL_EXT_geometry_shader4 : enable
#define EPS 1E-6

varying vec3 texcoord;
uniform float uSize;
uniform mat4 uMatrix;

void main() {
    mat4 tmatrix = uMatrix;
    vec2 p0 = (gl_PositionIn[0]).xy;
    vec2 p1 = (gl_PositionIn[1]).xy;
    vec2 dir = p1 - p0;
    texcoord.z = length(dir);

    if (texcoord.z > EPS) {
        dir = dir / texcoord.z;
    } else {
        dir = vec2(1.0, 0.0);
    }

    vec2 norm = vec2(-dir.y, dir.x);

    dir *= uSize;
    norm *= uSize;

    texcoord.xy = vec2(-uSize, -uSize);
    gl_Position = tmatrix * vec4(p0 - dir - norm, 0.0, 1.0);
    EmitVertex();

    texcoord.xy = vec2(-uSize,  uSize);
    gl_Position = tmatrix * vec4(p0 - dir + norm, 0.0, 1.0);
    EmitVertex();

    texcoord.xy = vec2(texcoord.z + uSize, -uSize);
    gl_Position = tmatrix * vec4(p1 + dir - norm, 0.0, 1.0);
    EmitVertex();

    texcoord.xy = vec2(texcoord.z + uSize,  uSize);
    gl_Position = tmatrix * vec4(p1 + dir + norm, 0.0, 1.0);
    EmitVertex();
}
