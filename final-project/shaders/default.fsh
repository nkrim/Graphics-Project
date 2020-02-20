#version 410

uniform vec4 vColor;
uniform vec3 fogColor;
uniform float fogDensity;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

uniform sampler2D water1Texture;
uniform sampler2D water2Texture;

uniform sampler2D heightFieldTexture;
uniform sampler2D waterMaskTexture;

uniform bool hasWaterMap;


uniform sampler2D grassDTexture;
uniform sampler2D gravelDTexture;

uniform vec3 direction;
uniform vec3 ambient;
uniform vec3 intensity;

uniform bool wireframe_mode;
uniform bool shadowing_mode;
uniform bool fog_mode;

uniform bool isReflection;

in vec2 f_tCoord;
in vec2 f_tCellCoord;
in vec3 f_camToPos;
in float f_timestamp;


uniform int levelOfDetail;

uniform float rain_t;

uniform bool isRain;

in float f_tDelt;
in vec2 f_tParentCoord;
uniform sampler2D parentTexture;
uniform bool blendTextures;







float PI = 3.14159265359;

uniform vec3 camLoc;


out vec4 fragColor;

float E = 2.71828182846;


vec3 detailTexture(sampler2D detail, vec3 in_color, float zoom, float start_fade, float end_fade) {
    float mask = 1-texture(detail, f_tCellCoord * zoom).g;
    vec3 color;
    if (length(f_camToPos) < start_fade)
        color = in_color * (1-mask) * 2;
    else {
        float t = clamp((length(f_camToPos) - start_fade) / end_fade, 0, 1);

        color = in_color * (1-mask) * (1-t)* 2 + in_color * t;
    }
    return color;
}


vec3 detailWater(vec3 color_in) {
    float deet = fract(f_timestamp);
    float dee2t = fract(f_timestamp/100);

    vec3 color1 = 0.2 + texture(water1Texture, f_tCoord*pow(2,levelOfDetail + 1) + dee2t + 0 * sin(deet * 2 * PI)/100).rgb;
    vec3 color2 = 0.2 + texture(water2Texture, f_tCoord*pow(2,levelOfDetail + 1) - dee2t - 0 * sin(deet * 2 * PI)/100).rgb;

    return (color1 * color2 + 0.4) * color_in;
}

void main () {
    vec3 color;

    // honestly though these aren't being used
    float height = texture(heightFieldTexture, f_tCellCoord).r;
    float water = texture(waterMaskTexture, f_tCellCoord).r;



    if (wireframe_mode) {
        color = vec3(vColor);
    } else {
        color = texture(diffuseTexture, f_tCoord).rgb;

        // Use parents texture if morphing to blend textures
        if(blendTextures) {
            vec3 parentTex = texture(parentTexture, f_tParentCoord).rgb;
            if(f_tDelt >= 0)   {
                color = (color * (1 - f_tDelt)) + (parentTex * f_tDelt);
            }
            else    {
                color = (color * -f_tDelt) + (parentTex * (1 + f_tDelt));
            }
        }

        if (color.b > color.g && color.b > color.r) {
            color = detailWater(color);
        } else if (color.g > color.b + 0.12 && color.g > color.r + 0.12) {
            color = detailTexture(grassDTexture, color, 50, 400, 800);
        } else {
            color = detailTexture(gravelDTexture, color, 20, 600, 800);
        }

    }

    bool shhhh = isRain;
  


    if (shadowing_mode) {
        vec3 in_normal = texture(normalTexture, f_tCoord).rgb;
        vec3 normal = vec3(in_normal.x, in_normal.z, in_normal.y);
        normal = (normal * 2) - 1;

        vec3 asdf = direction + ambient + intensity;

        fragColor = vec4( color * (max(0, dot(normalize(direction), normalize(normal))) * intensity),1);

    } else {
        fragColor = vec4(color, 1);
    }

    if (fog_mode && !wireframe_mode) {
        float fogd = fogDensity;
        // if (isRain) {
        fogd += rain_t * fogd * 2;
        // }
        float fogFactor = 1 / ( pow(E, pow( fogd *  length(f_camToPos), 2   )));
        fragColor = vec4(((1 - fogFactor) * fogColor), 1) + fogFactor * fragColor;
    }

    if (!wireframe_mode && hasWaterMap && ((!isReflection && water < 0.5) || (isReflection && !isReflection))) {
        if (!isReflection)
            discard;
    }


    if (rain_t > 0) {
        fragColor.xyz = fragColor.xyz * (1-rain_t/2) + vec3(.3,.3,.3) * (rain_t/2);
    }

}
