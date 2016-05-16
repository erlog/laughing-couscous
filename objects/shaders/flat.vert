//Inputs
attribute vec3 surface_tangent;
attribute vec3 surface_bitangent;
attribute vec3 surface_normal;

//Outputs
varying vec3 local_normal;

void main() {
    // Transforming The Vertex
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    local_normal = normalize(gl_NormalMatrix * surface_normal);
}
