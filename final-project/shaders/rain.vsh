#version 410

uniform mat4 modelView;
uniform mat4 projection;

uniform float timestamp;



layout (location = 0) in vec4 position;
layout (location = 1) in float t_off;
layout (location = 2) in float t_theta;
layout (location = 3) in float t_radii;

out vec2 f_tCoord;



uniform samplerBuffer TextureBufferObject;



void main () {
    vec3 pos = position.xyz;

    float which = pos[1];




    vec3 pos1 = texelFetch(TextureBufferObject, gl_VertexID).rgb;


    pos1 += pos;


    float t =fract(timestamp/3 + t_off);
    float vi = -30;
    float a = -400.0f;

    // pos1.x += 200*t_radii * cos(t_theta);
    // pos1.z += 200*t_radii * sin(t_theta);



    // pos1 = pos1 + offset;

    // pos1[0] += 2 * sin((timestamp + pos1[0] / 20 + pos1[2] / 60) * which / 2);


    // TODO pass in screen sizel
    float spriteSize = 8;

    vec2 screenSize = vec2(1024,768);

    vec4 relCamPos = modelView * vec4(pos1.xyz, 1.0);
    vec4 projVoxel = projection * vec4(spriteSize, spriteSize, relCamPos.z, relCamPos.w);
    vec2 projSize = screenSize * projVoxel.xy / projVoxel.w;

    gl_PointSize = 0.25 * (projSize.x + projSize.y);
    // gl_PointSize = spriteSize;

    gl_Position = projection * modelView * vec4(pos1.xyz,1.0);

    float test = timestamp;

}

