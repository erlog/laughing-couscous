#version 150
//Input
in vec2 texture_coordinate;

//Textures
uniform sampler2D diffuse;

//Variables
vec4 color;

//Output
out vec4 fragColor;

//Constants
const float smoothing = 1.0/16.0;

void main() {
    color = vec4(1.0, 1.0, 1.0, 1.0);
    float distance = texture2D(diffuse, texture_coordinate).a;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance) * color.a;
    fragColor = vec4(color.rgb, alpha);
}
