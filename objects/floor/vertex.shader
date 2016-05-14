//Inputs
attribute vec3 surface_tangent;
attribute vec3 surface_bitangent;
attribute vec3 surface_normal;

//Outputs
varying vec2 texture_coordinate;
varying vec3 local_tangent;
varying vec3 local_bitangent;
varying vec3 local_normal;

void main() {
    // Transforming The Vertex
    texture_coordinate = vec2(gl_MultiTexCoord0);
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    local_tangent = surface_tangent;
    local_bitangent = surface_bitangent;
    local_normal = normalize(surface_normal);
}
