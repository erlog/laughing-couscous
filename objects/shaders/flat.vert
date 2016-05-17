#version 120
//Inputs
attribute vec3 local_position;
attribute vec3 texture_coord;
attribute vec3 surface_normal;
attribute vec3 surface_tangent;
attribute vec3 surface_bitangent;

//Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//Outputs
varying vec3 local_normal;

void main() {
    mat4 vert_matrix = projection * view * model;
    gl_Position = vert_matrix * vec4(local_position, 1.0);
    
    mat3 normal_matrix = transpose(inverse(mat3(view * model)));
    local_normal = normal_matrix * surface_normal;
}
