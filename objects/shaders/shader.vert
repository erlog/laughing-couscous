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
out vec2 texture_coordinate;
out vec3 local_tangent;
out vec3 local_bitangent;
out vec3 local_normal;
out mat3 normal_matrix;

void main() {
    mat4 vert_matrix = projection * view * model;
    gl_Position = vert_matrix * vec4(local_position, 1.0);
    
    texture_coordinate = vec2(texture_coord.x, texture_coord.y);
    
    normal_matrix = transpose(inverse(mat3(view * model)));
    local_tangent = normalize(normal_matrix * surface_tangent);
    local_bitangent = normalize(normal_matrix * surface_bitangent);
    local_normal = normalize(normal_matrix * surface_normal);
}
