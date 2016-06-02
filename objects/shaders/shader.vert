#version 150
//Inputs
in vec3 local_position;
in vec3 texture_coord;
in vec3 surface_normal;
in vec3 surface_tangent;
in vec3 surface_bitangent;

uniform vec3 camera_direction;
uniform vec3 light_direction;
uniform mat4 model_view_projection;
uniform mat3 normal_matrix;

//Outputs
out vec2 texture_coordinate;
out vec3 local_tangent;
out vec3 local_bitangent;
out vec3 local_normal;

void main() {
    gl_Position = model_view_projection * vec4(local_position, 1.0);
    
    texture_coordinate = vec2(texture_coord.x, texture_coord.y);
    
    local_tangent = normalize(normal_matrix * surface_tangent);
    local_bitangent = normalize(normal_matrix * surface_bitangent);
    local_normal = normalize(normal_matrix * surface_normal);
}
