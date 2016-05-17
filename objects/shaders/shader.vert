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
varying vec2 texture_coordinate;
varying vec3 local_tangent;
varying vec3 local_bitangent;
varying vec3 local_normal;

void main() {
    mat4 vert_matrix = projection * view * model;
    gl_Position = vert_matrix * vec4(local_position, 1.0);
    
    texture_coordinate = vec2(texture_coord.x, texture_coord.y);
    
    mat3 normal_matrix = transpose(inverse(mat3(view * model)));
    local_tangent = normalize(normal_matrix * surface_tangent);
    local_bitangent = normalize(normal_matrix * surface_bitangent);
    local_normal = normalize(normal_matrix * surface_normal);
}
