#version 410

uniform mat4 modelView;
uniform mat4 projection;
uniform vec3 camPos;

uniform float timestamp;


layout (location = 0) in vec4 position;
layout (location = 1) in vec2 tCoord;
layout (location = 2) in vec3 offset;

out vec2 f_tCoord;
out vec3 f_Pos;

uniform samplerBuffer grassTBO;



void main () {
    vec3 pos = position.xyz;

    float which = pos[1];

    vec3 pos1 = texelFetch(grassTBO, gl_InstanceID).rgb;


    pos = pos + pos1;

    // pos = pos + offset;*

    pos[0] += 2 * sin((timestamp / 3 + pos[0] / 100 + pos[2] / 300) * which / 2);
    pos[1] += .5 * pow(abs(cos((timestamp / 3 + pos[0] / 100 + pos[2] / 300) * which / 2)),2);



    gl_Position = projection * modelView * vec4(pos.xyz,1.0);

    f_Pos = vec3(pos);
    f_tCoord = tCoord;

    float test = timestamp;

}

