#version 120
//Inputs
varying vec3 local_normal;

vec4 normal_to_color(vec3 normal) {
    normalize(normal);
    normal = (normal + 1.0)/2.0;
    float r = normal.x;
    float g = normal.y;
    float b = normal.z;
    return vec4(r,g,b,1.0);
}

void main() {
    //compute diffuse intensity
    vec3 light_direction = vec3(0.0, 0.0, 1.0);
    float diffuse_intensity = clamp(dot(light_direction, local_normal), 0.05, 1.0) * 1.15;
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); //* diffuse_intensity;
    gl_FragColor = normal_to_color(local_normal);
}
