#version 410

uniform sampler2D grassTexture;
uniform vec3 camPos;

in vec2 f_tCoord;
in vec3 f_Pos;



out vec4 fragColor;
// TODO: change grass texture to a narrower one.


void main () {

    vec4 color = vec4(texture(grassTexture, f_tCoord));



    if (color.w > 0) {
        // sample the grass so its smooth when far, not pixely
        vec4 color1 = vec4(texture(grassTexture, vec2(f_tCoord.x+.001, f_tCoord.y)));
        vec4 color2 = vec4(texture(grassTexture, vec2(f_tCoord.x-.001, f_tCoord.y)));
        vec4 color3 = vec4(texture(grassTexture, vec2(f_tCoord.x, f_tCoord.y+.001)));
        vec4 color4 = vec4(texture(grassTexture, vec2(f_tCoord.x, f_tCoord.y-.001)));

        fragColor = (color + color1+color2+color3+color4)/5;

    } else {
        discard;
    }

    float len = length(vec2(camPos.x, camPos.z) - vec2(f_Pos.x, f_Pos.z));

    float t = 1 - clamp( (len - 200) / 400, 0, 1);

    fragColor.w = fragColor.w * t;

}
