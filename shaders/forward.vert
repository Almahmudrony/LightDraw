#version 410
//name=light_forward

layout(location=0) in vec3 Position;
layout(location=1) in vec3 Normal;
layout(location=2) in vec2 Texcoord;

out vec2 var_texcoords;
out vec3 var_Normal;
out vec3 var_Position;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;


void main () {
    gl_Position = P * V * M * vec4(Position, 1);
    vec4 position_world = M * vec4(Position, 1);
    var_Position = position_world.xyz / position_world.w;


    var_texcoords = Texcoord;


    vec3 normal_world = N * Normal;
    var_Normal = normalize(normal_world);

}


