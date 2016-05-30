#version 150

//Inputs
in vec3 local_normal;
uniform vec4 matte_color;

//Outputs
out vec4 fragColor;

void main() {
    fragColor = matte_color;
}
