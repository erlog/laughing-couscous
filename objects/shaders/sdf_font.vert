#version 150
//Inputs
in vec3 local_position;
in vec3 texture_coord;

uniform vec4 char_info;
uniform mat4 model_view_projection;

//Outputs
out vec2 texture_coordinate;

void main() {
    gl_Position = model_view_projection * vec4(local_position, 1.0);
    texture_coordinate = vec2( char_info.x + (texture_coord.x * char_info.z),
        1.0 - (char_info.y + (texture_coord.y * char_info.w)) );
}
