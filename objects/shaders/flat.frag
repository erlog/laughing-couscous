#version 150

//Inputs
in vec3 local_normal;
uniform vec3 camera_direction;
uniform vec3 light_direction;
uniform vec4 matte_color;

//Outputs
out vec4 fragColor;

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
    float diffuse_intensity = clamp(dot(light_direction, local_normal)*-1.0, 0.25, 1.0) * 1.15;
    //fragColor = vec4(1.0, 0.0, 0.0, 1.0); // * diffuse_intensity;
    fragColor = matte_color * diffuse_intensity;
    //fragColor = normal_to_color(local_normal);
}
