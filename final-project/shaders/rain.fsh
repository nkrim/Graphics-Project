#version 410

uniform sampler2D rainTexture;




out vec4 fragColor;
// TODO: change grass texture to a narrower one.

void main () {

    float x = gl_PointCoord.x;

    vec4 color = vec4(texture(rainTexture, -gl_PointCoord));
    // color = vec4(1,1,1,1);
   
    if (color.w > 0.5) {

    	color.w *= 0.5;
        fragColor = color;
    } else {
        discard;
    }



}
