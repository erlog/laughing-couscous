#version 150
//Inputs
in vec3 local_position;
in vec3 texture_coord;
in vec3 surface_normal;
in vec3 surface_tangent;
in vec3 surface_bitangent;

//Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//Outputs
out vec3 local_normal;
out mat3 normal_matrix;

void main() {
    mat4 vert_matrix = projection * view * model;
    gl_Position = vert_matrix * vec4(local_position, 1.0);
    
    normal_matrix = transpose(inverse(mat3(view * model)));
    local_normal = normal_matrix * surface_normal;
}
