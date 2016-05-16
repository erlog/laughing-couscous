//Inputs
attribute vec3 local_position;
attribute vec3 texture_coord;
attribute vec3 surface_normal;
attribute vec3 surface_tangent;
attribute vec3 surface_bitangent;


//Outputs
varying vec2 texture_coordinate;
varying vec3 local_tangent;
varying vec3 local_bitangent;
varying vec3 local_normal;

void main() {
    // Transforming The Vertex
    texture_coordinate = vec2(texture_coord.x, texture_coord.y);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(local_position, 1.0);
    local_tangent = normalize(gl_NormalMatrix * surface_tangent);
    local_bitangent = normalize(gl_NormalMatrix * surface_bitangent);
    local_normal = normalize(gl_NormalMatrix * surface_normal);
}
