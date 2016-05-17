#version 150
//Inputs
in vec2 texture_coordinate;
in vec3 local_tangent;
in vec3 local_bitangent;
in vec3 local_normal;
in mat3 normal_matrix;

//Textures
uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

//Variables
vec3 light_direction;
vec3 camera_direction;
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
    light_direction = vec3(0.0, 0.0, 1.0);
    float diffuse_intensity = clamp(dot(light_direction, mapped_normal), 0.0, 1.0);

    //compute specular intensity
    camera_direction = normal_matrix * vec3(0.0, 0.0, 1.0);
    float factor = dot(mapped_normal, light_direction)*2.0;
    vec3 reflection_vector = (factor * mapped_normal) - light_direction;
    normalize(reflection_vector);

    color = texture(specular, texture_coordinate);
    float power = color.r*24.0;
    float reflectivity = clamp(dot(camera_direction, reflection_vector), 0.0, 1.0);
    reflectivity = pow(reflectivity, power);

    float intensity = 0.05 + 0.45*reflectivity + 0.6*diffuse_intensity;
    color = texture(diffuse, texture_coordinate) * intensity;
    fragColor = color;
}
