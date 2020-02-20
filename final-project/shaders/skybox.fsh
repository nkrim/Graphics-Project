#version 410

uniform sampler2D diffuseTexture;

uniform vec3 fogColor;
uniform int boxFace;
uniform bool hasFog;
uniform float alpha;

in vec3 f_pos;
in vec2 f_tCoord;

out vec4 fragColor;

uniform bool isRain;
uniform float rain_t;

float E = 2.71828182846;

void main () {
    vec3 color = texture(diffuseTexture, f_tCoord).rgb;


    // TODO; calculate horizon

    float t = clamp( (f_tCoord.y - 0.49) * 30   , 0.0, 1.0);
    if (hasFog && boxFace == 3) {
        fragColor = vec4(fogColor, 1);
    } else if (hasFog && boxFace != 2) {
        fragColor = (1-t) * vec4(fogColor, 1) +  (t) * vec4(color, 1);
    } else {
        fragColor = vec4(color, 1);
    }
    fragColor.a = alpha;

    if (rain_t > 0) {
        fragColor.xyz = fragColor.xyz * (1-rain_t/2) + vec3(.3,.3,.3) * (rain_t/2);
    }

    bool shhhh = isRain;
}
