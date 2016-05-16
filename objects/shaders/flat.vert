//Inputs
attribute vec3 local_position;
attribute vec3 texture_coord;
attribute vec3 surface_normal;
attribute vec3 surface_tangent;
attribute vec3 surface_bitangent;

//Outputs
varying vec3 local_normal;

void main() {
    // Transforming The Vertex
    gl_Position = gl_ModelViewMatrix * vec4(local_position, 1.0);
    local_normal = normalize(gl_NormalMatrix * surface_normal);
    //local_normal = vec3(0.0, 1.0, 0.0);
}
