#version 150
//Input
in vec2 texture_coordinate;

//Textures
uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

//Variables
vec4 color;

//Output
out vec4 fragColor;

void main() {
    color = texture(diffuse, texture_coordinate);
    if(color.r > 0) {
        fragColor = vec4(1.0, 1.0, 1.0, color.r);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
