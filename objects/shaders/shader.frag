#version 150
//Inputs
in vec2 texture_coordinate;
in vec3 local_tangent;
in vec3 local_bitangent;
in vec3 local_normal;
in mat3 normal_matrix;
uniform vec3 camera_direction;
uniform vec3 light_direction;

//Textures
uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

//Variables
vec3 mapped_normal;
vec4 color;

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
    //convert tangent normal to world normal;
    mat3 TBN = mat3(local_tangent, local_bitangent, local_normal);
    color = texture(normal, texture_coordinate);
    vec3 tangent_normal = normalize(vec3(color.rgb*2.0 - 1.0));
    mapped_normal = TBN * tangent_normal;

    //convert to world space
    normalize(mapped_normal);

    //compute diffuse intensity
    float diffuse_intensity = clamp(dot(light_direction, mapped_normal)*-1.0, 0.0, 1.0);

    //compute specular intensity
    vec3 reflection_vector = reflect(light_direction, mapped_normal);
    normalize(reflection_vector);

    color = texture(specular, texture_coordinate);
    float power = color.r*24.0;
    float reflectivity = clamp(dot(camera_direction, reflection_vector)*-1.0, 0.0, 1.0);
    reflectivity = pow(reflectivity, power);

    float intensity = 0.05 + 0.45*reflectivity + 0.6*diffuse_intensity;
    color = texture(diffuse, texture_coordinate) * intensity;
    color.a = 1.0;
    fragColor = color;
}
