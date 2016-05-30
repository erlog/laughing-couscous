#version 150
//Inputs
in vec3 local_position;
uniform mat4 model_view_projection;

void main() {
    gl_Position = model_view_projection * vec4(local_position, 1.0);
}
