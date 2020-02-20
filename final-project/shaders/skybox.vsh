#version 410

uniform mat4 modelView;
uniform mat4 projection;

uniform vec3 delta;

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 tCoord;

out vec2 f_tCoord;
out vec3 f_pos;

void main () {
    gl_Position = projection * modelView * vec4(position.xyz + delta,1.0);
    f_pos = position.xyz;
    f_tCoord = tCoord;
}

