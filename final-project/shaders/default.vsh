#version 410

uniform mat4 modelView;
uniform mat4 projection;
uniform vec3 toWorld;
uniform float tileWidth;
uniform float cellWidth;
uniform float tileNWC;
uniform float tileNWR;
uniform float tCoordMultOffset;
uniform float tCoordAddOffsetX;
uniform float tCoordAddOffsetZ;


// Morphing uniforms
uniform float tDelt;
uniform int tileNWCwrtP;
uniform int tileNWRwrtP;
uniform bool blendTextures;



uniform float t;

uniform float timestamp;

uniform vec4 scalingVector;
uniform vec3 camLoc;

layout (location = 0) in vec4 position;

out vec2 f_tCoord;

out vec2 f_tCellCoord;

out vec3 f_camToPos;
out float f_timestamp;



out float f_tDelt;
out vec2 f_tParentCoord;



void main () {



    float yDelt = 0;
    if(tDelt > 0 && tDelt < 1)   {
        yDelt = position.w * tDelt;
    }
    else if(tDelt < 0 && tDelt > -1)    {
        yDelt = position.w * (1 + tDelt);
    }
    if(blendTextures)    {
        f_tParentCoord.x = (((position.x - tileNWC) / tileWidth) * 0.5) + (tileNWCwrtP * 0.5);
        f_tParentCoord.y = (((position.z - tileNWR) / tileWidth) * 0.5) + (tileNWRwrtP * 0.5);
    }
    //yDelt *= 1000;
    f_tDelt = tDelt;








    vec4 pos = vec4(position.x * scalingVector.x,
                    (position.y + t * position.w) * scalingVector.y,
                    position.z * scalingVector.z,
                    1.0 * scalingVector.w) -
                vec4(camLoc.x - toWorld.x * scalingVector.x,
                     camLoc.y - toWorld.y * scalingVector.y,
                     camLoc.z - toWorld.z * scalingVector.z,
                     0.0);

    gl_Position = projection * modelView * pos;

    f_tCoord = vec2(
            (((position.x - tileNWC) / (tileWidth)) * tCoordMultOffset) + tCoordAddOffsetX,
            (((position.z - tileNWR) / (tileWidth)) * tCoordMultOffset) + tCoordAddOffsetZ
        );




    f_tCellCoord = vec2(
            position.x / (cellWidth ), // -1 ?????
            -position.z / (cellWidth )
        );





    float asdf = cellWidth;

    f_camToPos = pos.xyz;

    f_timestamp = timestamp;
}

